#include <MediaTrack.h>

#ifndef MEDIAPLUGIN_H
#define MEDIAPLUGIN_H

namespace BPrivate { namespace media {

class MediaPlugin
{
public:
						MediaPlugin();
	virtual				~MediaPlugin();
};

class Decoder;
class Reader;

} } // namespace BPrivate::media

using namespace BPrivate::media;

#endif
