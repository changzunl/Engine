#include "SpriteAnimDefinition.hpp"

#include "SpriteSheet.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

SpriteAnimDefinition::SpriteAnimDefinition(const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playbackType /*= SpriteAnimPlaybackType::LOOP*/)
	: m_spriteSheet((SpriteSheet*)&sheet)
	, m_startSpriteIndex(startSpriteIndex)
	, m_endSpriteIndex(endSpriteIndex)
	, m_durationSeconds(durationSeconds)
	, m_playbackType(playbackType)
{
}

SpriteAnimDefinition::SpriteAnimDefinition()
{
}

SpriteAnimDefinition::SpriteAnimDefinition(const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, SpriteAnimPlaybackType playbackType /*= SpriteAnimPlaybackType::LOOP*/, float secondsPerFrame /*= 1.0f / 12.0f*/)
	: m_spriteSheet((SpriteSheet*)&sheet)
	, m_startSpriteIndex(startSpriteIndex)
	, m_endSpriteIndex(endSpriteIndex)
	, m_playbackType(playbackType)
{
	m_durationSeconds = secondsPerFrame * (float)GetFrames();
}

bool SpriteAnimDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_startSpriteIndex = ParseXmlAttribute(element, "startFrame", 0);
	m_endSpriteIndex   = ParseXmlAttribute(element, "endFrame", 1);
	return true;
}

float SpriteAnimDefinition::GetDuration() const
{
	return m_durationSeconds;
}

const SpriteDefinition& SpriteAnimDefinition::GetSpriteDefAtTime(float seconds) const
{
	float secsPerFrame = m_durationSeconds / (float) GetFrames();

	int frame = (int) (seconds / secsPerFrame);
	int index = GetIndexAtFrame(frame);
	return m_spriteSheet->GetSpriteDef(m_startSpriteIndex + index);
}

int SpriteAnimDefinition::GetIndices() const
{
	return m_endSpriteIndex - m_startSpriteIndex + 1;
}

int SpriteAnimDefinition::GetFrames() const
{
	switch (m_playbackType)
	{
	case SpriteAnimPlaybackType::LOOP:          return GetIndices();
	case SpriteAnimPlaybackType::ONCE:          return GetIndices();
	case SpriteAnimPlaybackType::PINGPONG:      return GetIndices() * 2 - 2;
	}
	ERROR_AND_DIE("SpriteAnimPlaybackType case not handled!");
}

int SpriteAnimDefinition::GetIndexAtFrame(int frame) const
{
	switch (m_playbackType)
	{
	case SpriteAnimPlaybackType::LOOP:          return frame % GetFrames();
	case SpriteAnimPlaybackType::ONCE:          return MinInt(frame, GetFrames() - 1);
	case SpriteAnimPlaybackType::PINGPONG:      return GetIndexAtFramePingPong(frame % GetFrames());
	}
	ERROR_AND_DIE("SpriteAnimPlaybackType case not handled!");
}

int SpriteAnimDefinition::GetIndexAtFramePingPong(int frame) const
{
	const int maxIndex = GetIndices() - 1;
	if (frame <= maxIndex) return frame;

	return maxIndex - (frame - maxIndex);
}

const char* GetNameFromType(SpriteAnimPlaybackType type)
{
	static const char* const names[3] = { "once", "loop", "pingpong" };
	return names[(unsigned int)type];
}

SpriteAnimPlaybackType GetTypeByName(const char* name, SpriteAnimPlaybackType defaultType)
{
	static const SpriteAnimPlaybackType types[3] = { SpriteAnimPlaybackType::ONCE, SpriteAnimPlaybackType::LOOP, SpriteAnimPlaybackType::PINGPONG };
	for (SpriteAnimPlaybackType type : types)
	{
		if (_stricmp(GetNameFromType(type), name) == 0)
			return type;
	}
	return defaultType;
}
