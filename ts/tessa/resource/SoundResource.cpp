#include "Precompiled.h"
#include "ts/tessa/resource/SoundResource.h"
#include <szen/System/ErrorStream.hpp>

#include <szen/System/Config.hpp>

TS_PACKAGE1(resource)

SoundResource::SoundResource(const std::string &filepath) :
	ResourceBase(path)
{
}

SoundResource::~SoundResource()
{
	stopAll();
}

bool SoundResource::loadResource()
{
	TS_ASSERT(!m_loaded && "Sound sample has already been loaded");
	TS_ASSERT(!m_filename.empty() && "Must have file name");

	m_asset = new(std::nothrow) sf::SoundBuffer();
	TS_ASSERT(m_asset && "Allocation failed");

	if(!m_asset->loadFromFile(m_filename))
	{
		szerr << "Unable to open sound file: " << m_filename << ErrorStream::error;

		return false;
	}

	m_loaded = true;

	return true;
}

void SoundResource::updateChannels()
{
	for(auto it = m_soundChannels.begin(); it != m_soundChannels.end();)
	{
		if(it->second.getStatus() == sf::Sound::Stopped)
		{
			it = m_soundChannels.erase(it);
		}
		else
		{
			++it;
		}
	}
}

uint32 SoundResource::getActiveChannels()
{
	return m_soundChannels.size();
}

SoundChannel SoundResource::play(float volume)
{
	if(getActiveChannels() >= 25) return 0;

	SoundChannel channel = 1;
	if(!m_soundChannels.empty())
	{
		auto last = m_soundChannels.rbegin();
		channel = last->first + 1;
	}
	
	m_soundChannels.insert(std::make_pair(channel, sf::Sound(*m_asset)));

	m_soundChannels[channel].play();
	m_soundChannels[channel].setVolume(volume * (Config::getSoundVolume() / 100.f));

	return m_soundChannels.size();
}

void SoundResource::setLooping(SoundChannel channel, const bool looping)
{
	if(!isChannelActive(channel)) return;
	m_soundChannels[channel].setLoop(looping);
}

void SoundResource::setPitch(SoundChannel channel, float pitch)
{
	if(!isChannelActive(channel)) return;
	m_soundChannels[channel].setPitch(pitch);
}

void SoundResource::setPanning(SoundChannel channel, const sf::Vector3f position)
{
	if(!isChannelActive(channel)) return;
	m_soundChannels[channel].setPosition(position);
}

void SoundResource::pause(SoundChannel channel)
{
	if(!isChannelActive(channel)) return;

	m_soundChannels[channel].pause();
}

void SoundResource::pauseAll()
{
	for(auto it = m_soundChannels.begin(); it != m_soundChannels.end(); ++it)
	{
		it->second.pause();
	}
}

void SoundResource::stop(SoundChannel channel)
{
	if(!isChannelActive(channel)) return;

	m_soundChannels[channel].stop();
	m_soundChannels.erase(channel);
}

void SoundResource::stopAll()
{
	for(auto it = m_soundChannels.begin(); it != m_soundChannels.end(); ++it)
	{
		it->second.stop();
	}

	m_soundChannels.clear();
}

bool SoundResource::isPlaying()
{
	return !m_soundChannels.empty();
}

bool SoundResource::isPlaying(SoundChannel channel)
{
	const bool active = isChannelActive(channel);
	return !active || (active && m_soundChannels[channel].getStatus() == sf::Sound::Playing);
}

bool SoundResource::isChannelActive(SoundChannel channel)
{
	return m_soundChannels.find(channel) != m_soundChannels.end();
}

TS_END_PACKAGE1()
