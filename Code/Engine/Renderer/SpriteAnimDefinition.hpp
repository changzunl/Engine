#pragma once

class Shader;
class SpriteSheet;
class SpriteDefinition;

namespace tinyxml2
{
	class XMLElement;
}
typedef tinyxml2::XMLElement XmlElement;

enum class SpriteAnimPlaybackType
{
    ONCE,        // for 5-frame anim, plays 0,1,2,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4...
    LOOP,        // for 5-frame anim, plays 0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0...
    PINGPONG,    // for 5-frame anim, plays 0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1...
};

const char* GetNameFromType(SpriteAnimPlaybackType type);
SpriteAnimPlaybackType GetTypeByName(const char* name, SpriteAnimPlaybackType defaultType);

class SpriteAnimDefinition
{
public:
    SpriteAnimDefinition();
    SpriteAnimDefinition(const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP);
    SpriteAnimDefinition(const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP, float secondsPerFrame = 1.0f / 12.0f);

    bool LoadFromXmlElement(const XmlElement& element);

	float GetDuration() const;
	int GetFrames() const;

    const SpriteDefinition&   GetSpriteDefAtTime(float seconds) const;

private:                      
    int                       GetIndices() const;
    int                       GetIndexAtFrame(int frame) const;
    int                       GetIndexAtFramePingPong(int frame) const;

private:
    SpriteSheet*              m_spriteSheet         = nullptr;
    int                       m_startSpriteIndex    = -1;
    int                       m_endSpriteIndex      = -1;
    float                     m_durationSeconds     = 1.f;
    SpriteAnimPlaybackType    m_playbackType        = SpriteAnimPlaybackType::LOOP;

};

