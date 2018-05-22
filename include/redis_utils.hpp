#ifndef _REDIS_UTILS_HPP_
#define _REDIS_UTILS_HPP_

#include <hiredis/hiredis.h>

redisContext* redis_connect(std::string hostname, int port, bool timeout)
{
    redisContext* context;
    if (timeout)
    {
        struct timeval timeout = {1, 500000};
        context = redisConnectWithTimeout(hostname.c_str(), port, timeout);
    }
    else
    {
        context = redisConnect(hostname.c_str(), port);
    }
    return context;
}

redisReply* redis_command(redisContext* context, std::string command)
{
    redisReply* reply = (redisReply*)redisCommand(context, command.c_str());
    return reply;
}

unsigned char* redis_reply_to_image(redisReply* reply, int width, int height, int channels)
{
    unsigned int size = width * height;
    unsigned char* image  = new unsigned char[size * channels];
    int cpt = 0;
    for (int i = 0 ; i < size * 4; i++)
    {
        if (i%4 == 0)
        {
            continue;
        }
        image[cpt++] = reply->str[27 + i];
    }
    return image;
}

unsigned char* redis_get_image(redisContext* context, std::string key, int width, int height)
{
    std::string command = "GET " + key;
    redisReply* reply = redis_command(context, command);
    if (reply->type == REDIS_REPLY_NIL)
        return NULL;
    return redis_reply_to_image(reply, width, height, 3);
}

void redis_set_image(redisContext* context, std::string key, unsigned char* image, int width, int height, int channel)
{
    int size = width * height * channel;
    char* data = new char[size];
    for (int i = 0 ; i < size ; i++)
    {
        data[i] = (char)image[i];
    }
    // Use binary safe API to set image key
    redisReply* reply = (redisReply*)redisCommand(context, "set %b %b", key.c_str(), (size_t) key.length(), data, size);
}

#endif
