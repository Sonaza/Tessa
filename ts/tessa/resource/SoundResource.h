#pragma once

#include <sfml/Audio.hpp>
#include "ts/tessa/resource/ResourceBase.h"

#include <szen/System/Types.hpp>

#include <map>

TS_PACKAGE1(resource)

typedef uint32 SoundChannel;
typedef std::map<const SoundChannel, sf::Sound> SoundChannels;

class SoundResource : public ResourceBase<sf::SoundBuffer>
{
public:
	SoundResource(const std::string &filepath);
	~SoundResource();

	bool loadResource();

	void updateChannels();

	uint32 getActiveChannels();

	SoundChannel play(float volume = 100.f);

	void setLooping(SoundChannel channel, const bool looping);
	void setPitch(SoundChannel channel, float pitch);
	void setPanning(SoundChannel channel, const sf::Vector3f position);

	void pause(SoundChannel channel);
	void pauseAll();
	
	void stop(SoundChannel channel);
	void stopAll();

	bool isPlaying();
	bool isPlaying(SoundChannel channel);

	bool isChannelActive(SoundChannel channel);

private:

	SoundChannels	m_soundChannels;

};

TS_END_PACKAGE1()
