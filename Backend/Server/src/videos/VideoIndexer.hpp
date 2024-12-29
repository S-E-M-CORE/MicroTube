#ifndef VIDEOINDEXER_HPP
#define VIDEOINDEXER_HPP
#include "oatpp/macro/component.hpp"
#include "oatpp/Types.hpp"
#include "oatpp/json/ObjectMapper.hpp"
#include "oatpp/base/Log.hpp"

#include "dto/InfoJsonDTO.hpp"
#include "dto/VideoInfoTransferDTO.hpp"

#include "oatpp/web/protocol/http/Http.hpp"

#include "../sql/DatabaseClient.hpp"

#include <unordered_map>
#include <string>
#include <memory>
#include <filesystem>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cctype>

using Status = oatpp::web::protocol::http::Status;

namespace microTube
{
    namespace video
    {
        class VideoIndexer
        {
        public:
            using InfoDTO = microTube::dto::InfoJsonDTO;
            using UUIDList = oatpp::List<oatpp::String>;
            using VideoInfo = oatpp::Object<InfoDTO>;
            using VideoInfoMap = std::unordered_map<std::string, VideoInfo>;
            using VideoInfoList = std::list<VideoInfo>;
        public:
            VideoIndexer(const std::string& BasePath) : m_BasePath(BasePath), m_end(false)
            {
                m_worker = std::thread(IndexingThread, this);
            }

            ~VideoIndexer()
            {
                m_end = true;
                m_cv.notify_all();
                m_worker.join();
            }

            static void IndexingThread(VideoIndexer* const instance)
            {
                const size_t IndexIntervallMinutes = 5;

                auto& BasePath = instance->m_BasePath;
                auto& end = instance->m_end;
                auto& ObjectMapper = instance->m_ObjectMapper;
                auto& mtx = instance->m_videos.mtx;
                auto& videos = instance->m_videos.videos;

                while (!end)
                {
                    if (!std::filesystem::exists(BasePath) || !std::filesystem::is_directory(BasePath))
                    {
                        OATPP_LOGd("VideoIndexer", "\"{}\" does not exist or is not a directory", BasePath);
                        std::this_thread::sleep_for(std::chrono::minutes(2));
                        continue;
                    }

                    const size_t OldSize = videos.size();
                    auto temp = instance->getVideoMap();

                    mtx.lock();
                    videos.clear();
                    videos = std::move(temp.first);
                    mtx.unlock();

                    OATPP_LOGd("VideoIndexer", "Indexed {} videos ({} new).", BasePath, videos.size() - OldSize);
                    OATPP_LOGd("VideoIndexer", "Videos take {} Mb.", temp.second / 1000000);

                    std::unique_lock<std::mutex> lock(mtx);
                    if (instance->m_cv.wait_for(lock, std::chrono::minutes(IndexIntervallMinutes), [&end] { return end.load(); }))
                        break;
                }
            }

            std::pair<VideoInfoMap, uintmax_t> getVideoMap()
            {
                VideoInfoMap temp{};
                uintmax_t VideoSizeAll{ 0 };

                for (const auto& entry : std::filesystem::directory_iterator(this->m_BasePath))
                {
                    if (std::filesystem::is_directory(entry))
                    {
                        const std::string CurrentDir = entry.path().string() + "/";
                        oatpp::String InfoFileStr = oatpp::String::loadFromFile((CurrentDir + "info.json").c_str());

                        if (!InfoFileStr)
                            continue;

                        auto obj = m_ObjectMapper->readFromString<oatpp::Object<microTube::dto::InfoJsonDTO>>(InfoFileStr);

                        if (!obj)
                            continue;

                        VideoSizeAll += std::filesystem::file_size(CurrentDir + "video.mp4");

                        temp[entry.path().filename().string()] = obj;
                    }
                }
                return std::make_pair<>(std::move(temp), std::move(VideoSizeAll));
            }

            enum UUIDSort
            {
                UUIDSortDate,
                UUIDSortLikes,
                UUIDSortDislikes,
                UUIDSortUniqueViews
            };

            enum SortStyle
            {
                SortStyleASC,
                SortStyleDESC
            };

            void sortBy(const UUIDSort sortCategory, UUIDList& list, const SortStyle sortStyle = SortStyleASC)
            {
                switch (sortCategory)
                {
                case UUIDSortDate: SortDate(list, sortStyle);
                case UUIDSortLikes: SortLikes(list, sortStyle);
                case UUIDSortDislikes: SortDislikes(list, sortStyle);
                case UUIDSortUniqueViews: SortUniqueViews(list, sortStyle);
                default:;
                }
            }

        private:
            void SortDate(UUIDList& list, const SortStyle sortStyle)
            {
                std::lock_guard<std::mutex> lock(m_videos.mtx);
                list->sort([this, &sortStyle](const std::string& uuid1, const std::string& uuid2)
                    {
                        switch (sortStyle)
                        {
                        case SortStyleASC:  return *m_videos.videos[uuid1]->uploadDate < *m_videos.videos[uuid2]->uploadDate;
                        case SortStyleDESC: return *m_videos.videos[uuid1]->uploadDate > *m_videos.videos[uuid2]->uploadDate;
                        }
                        return false;
                    });
            }

            void SortLikes(UUIDList& list, const SortStyle sortStyle)
            {
                std::lock_guard<std::mutex> lock(m_videos.mtx);
                list->sort([this, &sortStyle](const std::string& uuid1, const std::string& uuid2)
                    {
                        switch (sortStyle)
                        {
                        case SortStyleASC:  return this->getLikeCountFor(uuid1) < this->getLikeCountFor(uuid2);
                        case SortStyleDESC: return this->getLikeCountFor(uuid1) > this->getLikeCountFor(uuid2);
                        }
                        return false;
                    });
            }

            void SortDislikes(UUIDList& list, const SortStyle sortStyle)
            {
                std::lock_guard<std::mutex> lock(m_videos.mtx);
                list->sort([this, &sortStyle](const std::string& uuid1, const std::string& uuid2)
                    {
                        switch (sortStyle)
                        {
                        case SortStyleASC:  return this->getDisikeCountFor(uuid1) < this->getDisikeCountFor(uuid2);
                        case SortStyleDESC: return this->getDisikeCountFor(uuid1) > this->getDisikeCountFor(uuid2);
                        }
                        return false;
                    });
            }

            void SortUniqueViews(UUIDList& list, const SortStyle sortStyle)
            {
                std::lock_guard<std::mutex> lock(m_videos.mtx);

                list->sort([this, &sortStyle](const std::string& uuid1, const std::string& uuid2)
                    {
                        switch (sortStyle)
                        {
                        case SortStyleASC:  return this->getUniqueViewCountFor(uuid1) < this->getUniqueViewCountFor(uuid2);
                        case SortStyleDESC: return this->getUniqueViewCountFor(uuid1) > this->getUniqueViewCountFor(uuid2);
                        }
                        return false;
                    });
            }

        public:

            oatpp::UInt64 getLikeCountFor(const std::string& uuid)
            {
                auto dbResult = m_database->getLikeCountForVideo(uuid);
                OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                auto ResultList = dbResult->fetch<oatpp::List<oatpp::List<oatpp::UInt64>>>();

                OATPP_ASSERT_HTTP(ResultList->size() == 1, Status::CODE_500, "Internal server error");

                return ResultList[0][0];
            }

            oatpp::UInt64 getDisikeCountFor(const std::string& uuid)
            {
                auto dbResult = m_database->getDislikeCountForVideo(uuid);
                OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                auto ResultList = dbResult->fetch<oatpp::List<oatpp::List<oatpp::UInt64>>>();

                OATPP_ASSERT_HTTP(ResultList->size() == 1, Status::CODE_500, "Internal server error");

                return ResultList[0][0];
            }

            oatpp::UInt64 getUniqueViewCountFor(const std::string& uuid)
            {
                auto dbResult = m_database->getUniqueViewCount(uuid);
                OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                auto ResultList = dbResult->fetch<oatpp::List<oatpp::List<oatpp::UInt64>>>();

                OATPP_ASSERT_HTTP(ResultList->size() == 1, Status::CODE_500, "Internal server error");

                return ResultList[0][0];
            }

            UUIDList getFullList()
            {
                UUIDList ret = UUIDList::createShared();
                m_videos.mtx.lock();
                for (auto& video : m_videos.videos)
                    if (!video.second->uuid.operator std::string().empty())
                        ret->push_back(video.second->uuid);
                m_videos.mtx.unlock();
                return std::move(ret);
            }

            UUIDList getChannelList(const oatpp::UInt64 channelId)
            {
                UUIDList ret = UUIDList::createShared();
                m_videos.mtx.lock();
                for (auto& video : m_videos.videos)
                    if (video.second->channelId == channelId)
                        ret->push_back(video.second->uuid);
                m_videos.mtx.unlock();
                return std::move(ret);
            }

            UUIDList getCategoryList(const oatpp::String& category)
            {
                UUIDList ret = UUIDList::createShared();
                m_videos.mtx.lock();
                for (auto& video : m_videos.videos)
                    for (auto& cat : *video.second->categories)
                        if (cat == category)
                        {
                            ret->push_back(video.second->uuid);
                            break;
                        }
                m_videos.mtx.unlock();
                return std::move(ret);
            }

            VideoInfoList getInfosFor(const UUIDList& list)
            {
                VideoInfoList ret;
                m_videos.mtx.lock();
                for (auto& uuid : *list)
                {
                    ret.push_back(this->m_videos.videos[uuid]);
                }
                m_videos.mtx.unlock();
                return std::move(ret);
            }

            VideoInfo lookupInfoManual(const std::string& uuid)
            {
                return lookupInfoManual(uuid, this->m_BasePath);
            }

            static VideoInfo lookupInfoManual(const std::string& uuid, const std::string& VideoDirectory)
            {
                OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, ObjectMapper);

                oatpp::String InfoJsonStr = oatpp::String::loadFromFile((VideoDirectory + "/" + uuid).c_str());

                if (!InfoJsonStr)
                    return InfoDTO::createShared();

                VideoInfo ret = ObjectMapper->readFromString<VideoInfo>(InfoJsonStr);

                return ret;
            }

            VideoInfo lookupInfo(const std::string& uuid)
            {
                m_videos.mtx.lock();
                VideoInfo ret = m_videos.videos[uuid];
                m_videos.mtx.unlock();
                return ret;
            }

            bool updateVideoInfo(const oatpp::Object<InfoDTO>& dto, const bool isNew = false)
            {
                std::lock_guard<std::mutex> lock(m_videos.mtx);

                if (!isNew)
                {
                    if (m_videos.videos.find(dto->uuid) == m_videos.videos.end())
                        return false;

                    oatpp::Object<InfoDTO> oldData = m_videos.videos[dto->uuid];
                    m_videos.videos[dto->uuid] = dto;
                    m_videos.videos[dto->uuid]->uploadDate = oldData->uploadDate;
                }
                else
                    m_videos.videos[dto->uuid] = dto;

                const oatpp::String JsonFile = m_ObjectMapper->writeToString(m_videos.videos[dto->uuid]);
                JsonFile.saveToFile((m_BasePath + "/" + dto->uuid + "/info.json")->c_str());
            }

            UUIDList searchFor(const std::string& str, const size_t amount = 15)
            {
                std::string searchStr = str;
                std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

                size_t pos = 0;
                while ((pos = searchStr.find('+', pos)) != std::string::npos)
                {
                    searchStr.replace(pos, 1, " ");
                    ++pos;
                }

                pos = 0;
                while ((pos = searchStr.find("%20", pos)) != std::string::npos)
                {
                    searchStr.replace(pos, 3, " ");
                    ++pos;
                }


                    auto searchIn = [&searchStr](const oatpp::String& text) -> bool
                    {
                        if (!text || text->size() == 0)
                            return false;

                        std::string textLower = text;
                        std::transform(textLower.begin(), textLower.end(), textLower.begin(), ::tolower);

                        return textLower.find(searchStr) != std::string::npos;
                    };

                std::list<std::pair<std::string, uint_fast16_t>> tempList;
                m_videos.mtx.lock();
                for (auto& video : m_videos.videos)
                {
                    uint_fast16_t weight = 0;

                    if (searchIn(video.second->uuid))
                        weight += 50;
                    if (searchIn(video.second->title))
                        weight += 40;
                    if (searchIn(video.second->description))
                        weight += 30;

                    if (video.second->categories)
                        for (auto& cat : *video.second->categories)
                            if (searchIn(cat))
                                weight += 30;

                    if (video.second->tags)
                        for (auto& tag : *video.second->tags)
                            if (searchIn(tag))
                                weight += 30;

                    if (searchIn(video.second->description))
                        weight += 30;

                    auto dbResult = m_database->getUserInfoById(video.second->userId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                    auto dbResultList = dbResult->fetch<oatpp::List<oatpp::Object<dto::UserInfoDTO>>>();
                    if (dbResultList->empty())
                        continue;
                    OATPP_ASSERT_HTTP(dbResultList->size() == 1, Status::CODE_500, "Internal server error");

                    auto& UserInfo = dbResultList[0];

                    if (searchIn(UserInfo->nickname))
                        weight += 100;
                    if (searchIn(UserInfo->firstname))
                        weight += 20;
                    if (searchIn(UserInfo->lastname))
                        weight += 20;
                    if (searchIn(UserInfo->email))
                        weight += 10;

                    dbResult = m_database->getChannelDescription(video.second->channelId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                    auto dbResultList2 = dbResult->fetch<oatpp::List<oatpp::List<oatpp::String>>>();
                    if (!dbResultList2->empty())
                    {
                        OATPP_ASSERT_HTTP(dbResultList2->size() == 1, Status::CODE_500, "Internal server error");

                        auto& ChannelDescription = dbResultList2[0][0];

                        if (searchIn(ChannelDescription))
                            weight += 10;
                    }

                    tempList.push_back(std::make_pair(video.second->uuid, weight));
                }
                m_videos.mtx.unlock();

                tempList.sort([](const std::pair<std::string, uint_fast16_t> vid1, const std::pair<std::string, uint_fast16_t> vid2)
                    {
                        return vid1.second > vid2.second;
                    });

                UUIDList ret = UUIDList::createShared();

                for (auto it = tempList.begin(); it != tempList.end() && ret->size() < amount; ++it)
                    ret->push_back(std::move(it->first));

                return ret;
            }


        private:
            OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, m_ObjectMapper);
            OATPP_COMPONENT(std::shared_ptr<microTube::component::DatabaseClient>, m_database);

            std::string m_BasePath;
            std::thread m_worker;


            struct
            {
                std::mutex    mtx;
                VideoInfoMap videos;
            } m_videos;
            std::atomic<bool> m_end;
            std::condition_variable m_cv;
        };

    } // namespace video
}     // namespace microTube


#endif // VIDEOINDEXER_HPP