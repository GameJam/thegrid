#include "NotificationLog.h"

#include "Particles.h"
#include "Utility.h"
#include "Font.h"
#include "Map.h"

const float kPi = 3.14159265359f;

const int kNumEntries = 4;

const char* kNotificationText[Protocol::Notification_Count] = {
    "Enemy agent captured!",
    "Your agent mysteriously disappeared!",
    "Incoming intel location tip",
    "Intel retrieved successfully!",
    "Suspicious activity reported",
    "Ticket purchased on ",
    "Enemy agent spotted!",
    "Safe house compromised!"
};

static bool NotificationBounceFunc(Particle& particle, float deltaTime)
{

    const float kDuration = 1.0;

    particle.time += deltaTime;
    if (particle.time > kDuration)
    {
        return false;
    }

    float scale = sinf(kPi * particle.time / kDuration);
    particle.scale = Vec2(scale, scale);
    particle.color = 0xffffff | (Clamp(static_cast<int>(255*scale), 0, 255) << 24);
    return true;

}

NotificationLog::NotificationLog(Map* map, Particles* mapParticles, Font* font)
{
    m_map = map;
    m_mapParticles = mapParticles;
    m_font = font;
    m_soundCrime = NULL;
    m_soundSpotted = NULL;
    m_soundDestroyed = NULL;
}

NotificationLog::~NotificationLog()
{
    BASS_SampleFree(m_soundCrime);
    BASS_SampleFree(m_soundSpotted);
    BASS_SampleFree(m_soundDestroyed);
}

void NotificationLog::Draw(int xSize, int ySize)
{
    size_t firstEntry = 0;
    if (m_entries.size() > kNumEntries)
    {
        firstEntry = m_entries.size() - kNumEntries;
    }

    const int fontHeight = Font_GetTextHeight(*m_font);
    const int rowSpacing = 8;

    int logWidth = xSize / 2;

    int rowY = ySize - (fontHeight + rowSpacing)*kNumEntries;

    Font_BeginDrawing(*m_font);
    glColor(0xff000000);
    for (size_t i = firstEntry; i < m_entries.size(); ++i)
    {
        Protocol::Notification notification = m_entries[i].packet.notification;
        const char* text = kNotificationText[notification];
        Font_DrawText(text, xSize - logWidth, rowY);
        
        if (notification == Protocol::Notification_LineUsed)
        {
            int xOffset = Font_GetTextWidth(*m_font, text);
            int line = m_entries[i].packet.line;
            char lineBuffer[1024];
            sprintf(lineBuffer, "line %i", line + 1);
            glColor(m_map->GetLineColor(line));
            Font_DrawText(lineBuffer, xSize - logWidth + xOffset, rowY);
            glColor(0xff000000);
        }

        rowY += fontHeight + rowSpacing;

    }
    Font_EndDrawing();

}

void NotificationLog::LoadResources()
{

    struct TextureLoad
    {
        Texture*    texture;
        const char* fileName;
    };

    TextureLoad load[] = 
    { 
        { &m_notificationAgentLost,                 "assets/notification_agent_lost.png"        },
        { &m_notificationAgentCaptured,             "assets/notification_agent_captured.png"    },
        { &m_notificationAgentSpotted,              "assets/notification_agent_spotted.png"     },
        { &m_notificationCrime,                     "assets/notification_crime.png"             },
        { &m_notificationBuildingDestroyed,         "assets/notification_building_destroyed.png" },
    };

    int numTextures = sizeof(load) / sizeof(TextureLoad);
    for (int i = 0; i < numTextures; ++i)
    {   
        Texture_Load(*load[i].texture, load[i].fileName);
    }

    m_soundCrime = BASS_SampleLoad(false, "assets/sound_crime.wav", 0, 0, 3, BASS_SAMPLE_OVER_POS);
    m_soundSpotted = BASS_SampleLoad(false, "assets/sound_spotted.wav", 0, 0, 3, BASS_SAMPLE_OVER_POS);
    m_soundDestroyed = BASS_SampleLoad(false, "assets/sound_infiltrate.wav", 0, 0, 3, BASS_SAMPLE_OVER_POS);

}

void NotificationLog::AddNotification(float time, const Protocol::NotificationPacket& packet)
{

    LogEntry entry = { time, packet };
    m_entries.push_back(entry);
    VisualizeNotification(packet);

}

void NotificationLog::PlaySample(HSAMPLE sample)
{
    HCHANNEL channel = BASS_SampleGetChannel(sample, false);
    BASS_ChannelPlay(channel, true);
}

void NotificationLog::VisualizeNotification(const Protocol::NotificationPacket& packet)
{

    switch (packet.notification)
    {
    case Protocol::Notification_AgentCaptured:
        {
            const Stop& stop = m_map->GetStop(packet.stop);        
            AddNotificationParticle(&m_notificationAgentCaptured, static_cast<int>(stop.point.x), static_cast<int>(stop.point.y));
        }
        break;
    case Protocol::Notification_AgentSpotted:
        {
            const Stop& stop = m_map->GetStop(packet.stop);        
            AddNotificationParticle(&m_notificationAgentSpotted, static_cast<int>(stop.point.x), static_cast<int>(stop.point.y));
            PlaySample(m_soundSpotted);
        }
        break;
    case Protocol::Notification_CrimeDetected:
        {
            const Stop& stop = m_map->GetStop(packet.stop);        
            AddNotificationParticle(&m_notificationCrime, static_cast<int>(stop.point.x), static_cast<int>(stop.point.y));\
            PlaySample(m_soundCrime);
        }
        break;
    case Protocol::Notification_AgentLost:
        {
            const Stop& stop = m_map->GetStop(packet.stop);        
            AddNotificationParticle(&m_notificationAgentLost, static_cast<int>(stop.point.x), static_cast<int>(stop.point.y));
        }
        break;
    case Protocol::Notification_HouseDestroyed:
        {
            const Stop& stop = m_map->GetStop(packet.stop);        
            AddNotificationParticle(&m_notificationBuildingDestroyed, static_cast<int>(stop.point.x), static_cast<int>(stop.point.y));
            PlaySample(m_soundDestroyed);
        }
        break;
    }

}

void NotificationLog::AddNotificationParticle(Texture* texture, int x, int y)
{
    Particle* p = m_mapParticles->Add();
    p->texture = texture;
    p->position = Vec2(x, y);
    p->scale = Vec2(0, 0);
    p->color = 0xffffffff;
    p->rotation = 0;
    p->updateFunction = NotificationBounceFunc;
}
