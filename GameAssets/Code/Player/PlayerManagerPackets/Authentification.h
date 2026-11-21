#pragma once
enum class PlayerAuthType 
{
	None,
	SelfSign,
	OnlineKey
};
namespace PlayerAuthRejectStatus 
{
	const unsigned char ok 				= 0;
	const unsigned char uuidFormat 		= 1;
	const unsigned char usernameFormat 	= 2;
	const unsigned char keyFormat 		= 3;
	const unsigned char credentials		= 4;
	const unsigned char uuidExists		= 5;
	const unsigned char usernameExists 	= 6;
	const unsigned char peerLoggedIn	= 7;
}
static const unsigned int usernameSize = 128;
static const unsigned int uuidSize = 64;
static const unsigned int keySize = 128;

#pragma pack(push, 1)
struct PlayerAuthentificatePacket
{
	unsigned char packetType;
	char username[usernameSize];
	char key[keySize];
	char uuid[uuidSize];
};

struct PlayerAuthenrificateResponsePacket 
{
	unsigned char packetType;
	unsigned char reject;
};
#pragma pack(pop)