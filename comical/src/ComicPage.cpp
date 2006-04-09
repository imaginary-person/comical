//
// C++ Implementation: ComicPage
//
// Description: 
//
//
// Author: James Leighton Athey <jathey@comcast.net>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <cstring>
#include "ComicPage.h"
#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!
#include "Exceptions.h"

WX_DEFINE_OBJARRAY(ArrayPage);

ComicPage::ComicPage(ComicBook *theBook, wxString &filename) : Filename(filename)
{
	book = theBook;
	Orientation = NORTH;
	Width = 0;
	Height = 0;
}

ComicPage::~ComicPage()
{
	if (Original.Ok())
		Original.Destroy();
	if (Resample.Ok())
		Resample.Destroy();
	if (Thumbnail.Ok())
		Thumbnail.Destroy();
}

void ComicPage::Rotate(COMICAL_ROTATE direction)
{
	if (Orientation != direction) {
		wxMutexLocker rlock(ResampleLock);
		wxMutexLocker tlock(ThumbnailLock);
		Orientation = direction;
		Resample.Destroy();
		Thumbnail.Destroy();
	}
}

#define STREAM_READ(stream, buffer, length) if(!(stream->Read(buffer, length).LastRead())) goto error;
#define STREAM_SEEK(stream, pos, mode) if(stream->SeekI(pos, mode) == wxInvalidOffset) goto error;

bool ComicPage::ExtractDimensions()
{
	wxUint8 buffer[8];
	wxUint8 jpegHeader[] = { 0xFF, 0xD8 };
	wxUint8 tiffBigHeader[] = { 0x4d, 0x4d, 0x00, 0x2a };
	wxUint8 tiffLittleHeader[] = { 0x49, 0x49, 0x2a, 0x00 };
	wxUint8 gif87Header[] = { 'G', 'I', 'F', '8', '7', 'a' };
	wxUint8 gif89Header[] = { 'G', 'I', 'F', '8', '9', 'a' };
	wxUint8 pngHeader[] = { '\211', 'P', 'N', 'G', '\r', '\n', '\032', '\n' };
	wxUint16 width16, height16, length;

	wxInputStream *stream;
	try {
		stream = book->ExtractStream(Filename);
	} catch (ArchiveException *ae) {
		return false;
	}
	if (!stream->IsOk() || !stream->CanRead())
		goto error;
	if(Filename.Right(5).Upper() == wxT(".JPEG") || Filename.Right(4).Upper() == wxT(".JPG")) {
		char c;
		STREAM_READ(stream, buffer, 2)
		if (memcmp(buffer, jpegHeader, 2))
			goto error;
		while (1) {
			// Find 0xFF byte; count and skip any non-FFs.
			while ((c = stream->GetC()) != 0xFF)
				if (!stream->LastRead()) goto error;
			// Get marker code byte, swallowing any duplicate FF bytes.
			while ((c = stream->GetC()) == 0xFF)
				if (!stream->LastRead()) goto error;
			
			if (c >= 0xC0 || c <= 0xCF)
				break;
			
			// Skip this marker
			STREAM_READ(stream, &length, 2)
			STREAM_SEEK(stream, wxUINT16_SWAP_ON_LE(length) - 2, wxFromCurrent)
		}
		STREAM_SEEK(stream, 3, wxFromCurrent)
		STREAM_READ(stream, &height16, 2)
		STREAM_READ(stream, &width16, 2)
		Width = wxUINT16_SWAP_ON_LE(width16);
		Height = wxUINT16_SWAP_ON_LE(height16);
	} else if (Filename.Right(4).Upper() == wxT(".GIF")) {
		STREAM_READ(stream, buffer, 6)
		if (!memcmp(buffer, gif87Header, 6) || !memcmp(buffer, gif89Header, 6))
			goto error;
		STREAM_READ(stream, &width16, 2)
		STREAM_READ(stream, &height16, 2)
		Width = wxUINT16_SWAP_ON_BE(width16);
		Height = wxUINT16_SWAP_ON_BE(height16);
	} else if (Filename.Right(4).Upper() == wxT(".PNG")) {
		STREAM_READ(stream, buffer, 8)
		if (memcmp(buffer, pngHeader, 8))
			goto error;
		STREAM_SEEK(stream, 8, wxFromCurrent)
		STREAM_READ(stream, &Width, 4)
		STREAM_READ(stream, &Height, 4)
		Width = wxUINT32_SWAP_ON_LE(Width);
		Height = wxUINT32_SWAP_ON_LE(Height);
	} else if (Filename.Right(5).Upper() == wxT(".TIFF") || Filename.Right(4).Upper() == wxT(".TIF")) {
		wxUint16 count, tag, type;
		wxUint32 offset;
		STREAM_READ(stream, buffer, 4)
		// Since TIFF supports both byte orders, there's alot of code duplication below.
		if (!memcmp(buffer, tiffBigHeader, 4)) { // Big Endian TIFF
			STREAM_READ(stream, &offset, 4)
			offset = wxUINT32_SWAP_ON_LE(offset);
			STREAM_SEEK(stream, offset, wxFromStart)
			STREAM_READ(stream, &count, 2)
			count = wxUINT16_SWAP_ON_LE(count);
			for (wxUint16 i = 0; i < count; i++) {
				STREAM_READ(stream, &tag, 2)
				tag = wxUINT16_SWAP_ON_LE(tag);
				if (tag == 256) { // ImageWidth
					STREAM_READ(stream, &type, 2)
					type = wxUINT16_SWAP_ON_LE(type);
					STREAM_SEEK(stream, 4, wxFromCurrent) // there's only going to be 1 of these
					if (type == 3) { // SHORT
						STREAM_READ(stream, &width16, 2)
						Width = wxUINT16_SWAP_ON_LE(width16);
					} else if (type == 4) { // LONG
						STREAM_READ(stream, &Width, 4)
						Width = wxUINT32_SWAP_ON_LE(Width);
					}
				} else if (tag == 257) { // ImageHeight
					STREAM_READ(stream, &type, 2)
					type = wxUINT16_SWAP_ON_LE(type);
					STREAM_SEEK(stream, 4, wxFromCurrent) // there's only going to be 1 of these
					if (type == 3) { // SHORT
						STREAM_READ(stream, &height16, 2)
						Height = wxUINT16_SWAP_ON_LE(height16);
					} else if (type == 4) { // LONG
						STREAM_READ(stream, &Height, 4)
						Height = wxUINT32_SWAP_ON_LE(Height);
					}
				}
				if (Width && Height) // i.e., if Width and Height have both been set
					break;
			}
		} else if (!memcmp(buffer, tiffLittleHeader, 4)) { // Little Endian TIFF
			STREAM_READ(stream, &offset, 4)
			offset = wxUINT32_SWAP_ON_BE(offset);
			STREAM_SEEK(stream, offset, wxFromStart)
			STREAM_READ(stream, &count, 2)
			count = wxUINT16_SWAP_ON_LE(count);
			for (wxUint16 i = 0; i < count; i++) {
				STREAM_READ(stream, &tag, 2)
				tag = wxUINT16_SWAP_ON_BE(tag);
				if (tag == 256) { // ImageWidth
					STREAM_READ(stream, &type, 2)
					type = wxUINT16_SWAP_ON_BE(type);
					STREAM_SEEK(stream, 4, wxFromCurrent) // there's only going to be 1 of these
					if (type == 3) { // SHORT
						STREAM_READ(stream, &width16, 2)
						Width = wxUINT16_SWAP_ON_BE(width16);
					} else if (type == 4) { // LONG
						STREAM_READ(stream, &Width, 4)
						Width = wxUINT32_SWAP_ON_BE(Width);
					}
				} else if (tag == 257) { // ImageHeight
					STREAM_READ(stream, &type, 2)
					type = wxUINT16_SWAP_ON_LE(type);
					STREAM_SEEK(stream, 4, wxFromCurrent) // there's only going to be 1 of these
					if (type == 3) { // SHORT
						STREAM_READ(stream, &height16, 2)
						Height = wxUINT16_SWAP_ON_BE(height16);
					} else if (type == 4) { // LONG
						STREAM_READ(stream, &Height, 4)
						Height = wxUINT32_SWAP_ON_BE(Height);
					}
				}
				if (Width && Height) // i.e., if Width and Height have both been set
					break;
			}
		} else
			goto error;
	} else {
		error:
		// Memory Input Streams don't take ownership of the buffer
		wxMemoryInputStream *mstream = dynamic_cast<wxMemoryInputStream*>(stream);
		if (mstream)
			delete[] (wxUint8 *) mstream->GetInputStreamBuffer()->GetBufferStart();
		wxDELETE(stream);
		return false;
	}

	// Memory Input Streams don't take ownership of the buffer
	wxMemoryInputStream *mstream = dynamic_cast<wxMemoryInputStream*>(stream);
	if (mstream)
		delete[] (wxUint8 *) mstream->GetInputStreamBuffer()->GetBufferStart();
	wxDELETE(stream);
	return true;
	
}