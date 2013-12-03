/*
 * LAV_Source: use FFmpeg/Libav to extract audio for Last.fm fingerprinting
 * Copyright (C) 2012 John Stamp <jstamp@mehercule.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LAV_SOURCE_H
#define LAV_SOURCE_H

#include <lastfm/FingerprintableSource.h>

using namespace std;

class LAV_Source : public lastfm::FingerprintableSource
{
public:
    LAV_Source();
    ~LAV_Source();

    void getInfo(int& lengthSecs, int& samplerate, int& bitrate, int& nchannels);
    void init(const QString& fileName);
    void release();

    int updateBuffer(signed short* pBuffer, size_t bufferSize);

    void skip(const int mSecs);
    void skipSilence(double silenceThreshold = 0.0001);

    bool eof() const;

private:
    class LAV_SourcePrivate * const d;
};

#endif
