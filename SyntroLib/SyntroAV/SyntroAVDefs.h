//
//  Copyright (c) 2012 Pansenti, LLC.
//	
//  This file is part of SyntroLib
//
//  SyntroLib is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  SyntroLib is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with SyntroLib.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _SYNTROAVDEFS_H
#define _SYNTROAVDEFS_H

#include "SyntroDefs.h"

//  Record header parameter field values

typedef enum
{
    SYNTRO_RECORDHEADER_PARAM_NOOP = 0,                     // indicates a filler record
    SYNTRO_RECORDHEADER_PARAM_NORMAL_START,                 // indicates a start normal record
    SYNTRO_RECORDHEADER_PARAM_NORMAL,                       // indicates a normal record
    SYNTRO_RECORDHEADER_PARAM_PREROLL_START,                // indicates start preroll frame
    SYNTRO_RECORDHEADER_PARAM_PREROLL,                      // indicates a preroll frame
    SYNTRO_RECORDHEADER_PARAM_POSTROLL_START,               // indicates a start postroll frame
    SYNTRO_RECORDHEADER_PARAM_POSTROLL,                     // indicates a postroll frame
 } SYNTROAV_RECORDHEADER_PARAM;

//	Video subType codes
typedef enum
{
	SYNTRO_RECORD_TYPE_VIDEO_UNKNOWN = -1,					// unknown format
	SYNTRO_RECORD_TYPE_VIDEO_MJPEG,							// MJPEG compression
	SYNTRO_RECORD_TYPE_VIDEO_MPEG1,							// MPEG1 compression
	SYNTRO_RECORD_TYPE_VIDEO_MPEG2,							// MPEG2 compression
	SYNTRO_RECORD_TYPE_VIDEO_H264,							// H264 compression

	// This entry marks the end of the enum

	SYNTRO_RECORD_TYPE_VIDEO_END							// the end
} SYNTROAV_VIDEOSUBTYPE;

//	SYNTRO_RECORD_VIDEO - used to send video frames
//
//	The structure follows the SYNTRO_EHEAD structure. Immediately following this structure
//	is the left (or single) image data followed by the right image data (if present).

typedef struct
{
	SYNTRO_RECORD_HEADER recordHeader;						// the record type header
	SYNTRO_UC2 width;										// width of each image
	SYNTRO_UC2 height;										// height of each image
	SYNTRO_UC4 size;										// size of the image
} SYNTRO_RECORD_VIDEO;

//	Audio subType codes
typedef enum
{
	SYNTRO_RECORD_TYPE_AUDIO_UNKNOWN = -1,					// unknown format
	SYNTRO_RECORD_TYPE_AUDIO_PCM,							// PCM/WAV uncompressed
	SYNTRO_RECORD_TYPE_AUDIO_MP3,							// MP3 compression
	SYNTRO_RECORD_TYPE_AUDIO_AC3,							// AC3 compression
	SYNTRO_RECORD_TYPE_AUDIO_AAC,							// AAC compression

	// This entry marks the end of the enum

	SYNTRO_RECORD_TYPE_AUDIO_END							// the end
} SYNTROAV_AUDIOSUBTYPE;

//	SYNTRO_RECORD_AUDIO - used to send audio data
//
//	The structure follows the SYNTRO_EHEAD structure. Immediately following this structure
//	is the audio data.

typedef struct
{
	SYNTRO_RECORD_HEADER recordHeader;						// the record type header
	SYNTRO_UC4 size;										// size of the audio data
	SYNTRO_UC4 bitrate;										// bitrate of audio
	SYNTRO_UC4 sampleRate;									// sample rate of audio
	SYNTRO_UC2 channels;									// number of channels
	SYNTRO_UC2 sampleSize;									// size of samples (in bits)
} SYNTRO_RECORD_AUDIO;


//	SYNTRO_PTZ - used to send PTZ information
//
//	Data is sent in servo form, i.e. 1 (SYNTRO_SERVO_LEFT) to 0xffff (SYNTRO_SERVO_RIGHT).
//	SYNTRO_SERVER_CENTER means center camera in that axis. For zoom, it menas some default position.

typedef struct
{
	SYNTRO_UC2 pan;											// pan position
	SYNTRO_UC2 tilt;										// tilt position
	SYNTRO_UC2 zoom;										// zoom position
} SYNTRO_PTZ;



#endif	// _SYNTROAVDEFS_H

