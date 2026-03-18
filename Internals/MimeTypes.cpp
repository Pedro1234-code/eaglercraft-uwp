
#include "pch.h"

#include "MimeTypes.h"

typedef std::map<std::string, std::string> MimeTypeMap;

const MimeTypeMap mimeTypes = {

	// Basic text-based web formats
	{ ".html",		"text/html; charset=utf-8" },
	{ ".htm",		"text/html; charset=utf-8" },
	{ ".js",		"application/javascript; charset=utf-8" },
	{ ".ts",		"application/typescript; charset=utf-8" },
	{ ".css",		"text/css; charset=utf-8" },
	{ ".json",		"application/json; charset=utf-8" },
	{ ".xml",		"text/xml; charset=utf-8" },
	{ ".csv",		"text/csv; charset=utf-8" },
	{ ".tsv",		"text/tab-separated-values; charset=utf-8" },
	{ ".txt",		"text/plain; charset=utf-8" },
	{ ".md",		"text/markdown; charset=utf-8" },
	{ ".ini",		"text/plain; charset=utf-8" },

	// WebAssembly
	{ ".wasm",		"application/wasm" },
	{ ".wast",		"application/wast" },

	// Image formats
	{ ".png",		"image/png" },
	{ ".apng",		"image/apng" },
	{ ".jpg",		"image/jpeg" },
	{ ".jpeg",		"image/jpeg" },
	{ ".jxl",		"image/jxl" },
	{ ".svg",		"image/svg+xml" },
	{ ".webp",		"image/webp" },
	{ ".gif",		"image/gif" },
	{ ".tif",		"image/tiff" },
	{ ".tiff",		"image/tiff" },
	{ ".bmp",		"image/bmp" },
	{ ".ico",		"image/x-icon" },
	{ ".avif",		"image/avif" },

	// Audio/video formats
	{ ".webm",		"video/webm" },		// could be video and/or audio; prefer video type
	{ ".m4a",		"audio/mp4" },
	{ ".mp3",		"audio/mpeg" },
	{ ".mp4",		"video/mp4" },
	{ ".mpg",		"video/mpeg" },
	{ ".mpeg",		"video/mpeg" },
	{ ".opus",		"audio/ogg; codecs=opus" },
	{ ".ogg",		"audio/ogg" },
	{ ".ogv",		"video/ogg" },
	{ ".flac",		"audio/flac" },
	{ ".wav",		"audio/wav" },
	{ ".mid",		"audio/midi" },
	{ ".midi",		"audio/midi" },
	{ ".kar",		"audio/midi" },

	// Font formats
	{ ".woff",		"application/font-woff" },
	{ ".woff2",		"font/woff2" },
	{ ".ttf",		"application/font-sfnt" },
	{ ".otf",		"application/font-sfnt" },
	{ ".eot",		"application/vnd.ms-fontobject" },

	// Misc. other
	{ ".zip",		"application/zip" },
	{ ".pdf",		"application/pdf" },
	{ ".scml",		"text/xml" },				// for Spriter
	{ ".scon",		"application/json" }		// for Spriter

	// Note: all others will default to application/octet-stream
};

std::string GetMimeTypeForFilename(const std::string& filename)
{
	std::string ext = ToLower(GetFileExtension(filename));
	MimeTypeMap::const_iterator i = mimeTypes.find(ext);

	if (i == mimeTypes.end())
	{
		return "application/octet-stream";
	}
	else
	{
		return i->second;
	}
}