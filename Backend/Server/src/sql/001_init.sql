-- User Table
CREATE TABLE User(
	id	            INTEGER PRIMARY KEY,
	email           VARCHAR(100),
	password        VARCHAR(500),
	nickname	    VARCHAR(100),
	firstname       VARCHAR(100),
	lastname        VARCHAR(100),
	phonenumber	    VARCHAR(100),
    birthDate       DATE,
    joinDate        DATE,
	secretQuestion1	VARCHAR(100),
	secretQuestion2	VARCHAR(100),
	secretQuestion3	VARCHAR(100),
	secretAnswer1   VARCHAR(100),
	secretAnswer2   VARCHAR(100),
	secretAnswer3   VARCHAR(100),
	isAdmin         BOOLEAN,
	isActive		BOOLEAN
);

-- Channel Table
CREATE TABLE Channel(
	id				INTEGER PRIMARY KEY,
	description		VARCHAR(1000)
);

-- Channel/User Junction
CREATE TABLE UserChannelJunction(
	channelId	INTEGER,
	user_id		INTEGER,
	PRIMARY KEY (channelId, user_id),
	FOREIGN KEY (channelId) REFERENCES Channel(id),
	FOREIGN KEY (user_id)   REFERENCES User(id)
);


-- Likes
CREATE TABLE Video_Likes(
	uuid		VARCHAR(100),
	user_id		INTEGER,
	PRIMARY KEY (uuid, user_id),
	FOREIGN KEY (user_id) REFERENCES User(id)
);

-- Dislikes
CREATE TABLE Video_Disikes(
	uuid		VARCHAR(100),
	user_id		INTEGER,
	PRIMARY KEY (uuid, user_id),
	FOREIGN KEY (user_id) REFERENCES User(id)
);

-- Unique Views
CREATE TABLE Videos_Unique_Views(
	uuid		VARCHAR(100),
	user_id		INTEGER,
	PRIMARY KEY (uuid, user_id),
	FOREIGN KEY (user_id) REFERENCES User(id)
);

-- Subscriptions
CREATE TABLE User_Channel_Subscriptions(
	memberID	INTEGER,
	creatorID	INTEGER,
	PRIMARY KEY (memberID, creatorID),
	FOREIGN KEY (memberID) REFERENCES User(id),
	FOREIGN KEY (creatorID) REFERENCES Channel(id)
);

-- Bearer
CREATE TABLE User_Tokens(
	token			VARCHAR(100),
	user_id		    INTEGER,
    createDate      DATE,
    validTillDate   DATE,
	PRIMARY KEY (token, user_id),
	FOREIGN KEY (user_id) REFERENCES User(id)
);