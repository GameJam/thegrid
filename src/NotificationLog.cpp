#include "NotificationLog.h"

#include "Particles.h"
#include "Utility.h"
#include "Font.h"
#include "Map.h"

const float kPi = 3.14159265359f;

const int kNumEntries = 4;

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
}

NotificationLog::~NotificationLog()
{
    BASS_SampleFree(m_soundCrime);
}

void NotificationLog::Draw(int xSize, int ySize)
{
    const size_t firstEntry = Min<size_t>(0, m_entries.size() - kNumEntries);

    const int fontHeight = Font_GetTextHeight(*m_font);
    const int rowSpacing = 8;

    int logWidth = xSize / 3;

    int rowY = ySize - (fontHeight + rowSpacing)*kNumEntries;

    Font_BeginDrawing(*m_font);
    glColor(0xff000000);
    for (size_t i = firstEntry; i < m_entries.size(); ++i)
    {
        char buffer[1024];
        sprintf(buffer, "%.2f: Alert %d", m_entries[i].time, m_entries[i].packet.notification);
        Font_DrawText(buffer, xSize - logWidth, rowY);
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
    };

    int numTextures = sizeof(load) / sizeof(TextureLoad);
    for (int i = 0; i < numTextures; ++i)
    {   
        Texture_Load(*load[i].texture, load[i].fileName);
    }

    m_soundCrime = BASS_SampleLoad(false, "assets/sound_crime.wav", 0, 0, 3, BASS_SAMPLE_OVER_POS);

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
