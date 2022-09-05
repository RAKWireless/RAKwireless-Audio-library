#author：Clark Li
#date：2022.08.19

import wave
import os

class wavFile:

    def __init__(self, filename):
        self.filename = filename
        self.file = wave.open(filename, 'rb')
        self.frameCount = self.file.getnframes()

    def wavFileName(self):
        return self.filename

    def describe(self):
        return self.file.getparams()

    def close(self):
        return self.file.close()

    def wavLength(self):
        count = self.file.getnframes()
        channelse = self.file.getnchannels()
        sampwidth = self.file.getsampwidth()
        return count*channelse*sampwidth+44

    def wavDataLength(self):
        count = self.file.getnframes()
        channelse = self.file.getnchannels()
        sampwidth = self.file.getsampwidth()
        return count*channelse*sampwidth

    def wavChannels(self):
        return  self.file.getnchannels()

    def wavSampwidth(self):
        return self.file.getsampwidth()

    def byteArray(self):
        curFrame = 0
        frameArray = []
        # print (self.frameCount)
        c = self.wavChannels()
        b = self.wavSampwidth()
        arryElement = b*c
        # print("arryElement:%d"% arryElement)
        while curFrame < self.frameCount:
            frame = self.file.readframes(1)
            # print("%x,%x,%x,%x"%(frame[0],frame[1],frame[2],frame[3]))
            # print("%x"%frame[1])
            # print("%x"%frame[2])
            # print("%x"%frame[3])

            for i in range(0,arryElement,1):
                frameArray.append(frame[i])

            # frameArray.append(frame[0])
            # frameArray.append(frame[1])
            # frameArray.append(frame[2])
            # frameArray.append(frame[3])
            # print("%x" % b)
            curFrame += 1
        return frameArray

    def byteFormatedString(self, bArray):
        bStrings = []
        for idx, val in enumerate(bArray):
            hexStr = ''
            if ((idx % 16 == 0) and (idx > 0)):
                hexStr += '\n'
            hexStr += ('0x{:02x}'.format(val))
            bStrings.append(hexStr)
        return  ",".join(bStrings)

    def writeFile(self):
        txtName = self.filename.replace('.wav', '.h')
        txt = open(txtName, 'w')
        txt.truncate()

        txt.write('#include "%s" \n' % (txtName))

        txt.write('\n\n//Auto generated File:%s \n'% self.wavFileName())

        wavParams = self.describe()
        print(wavParams)

        nChannels = wavParams[0]
        # print("nChannels:%d" % nChannels)

        if nChannels == 2:
            # print("nChannels:Stereo")
            txt.write('//channels:Stereo\n')
        else:
            # print("nChannels:Mono")
            txt.write('//channels:Mono\n')

        nSampwidth = wavParams[1]
        # print("nSampwidth:%d" % nSampwidth*8)
        txt.write('//Sample bits:%d\n'% (nSampwidth*8))
        nsampRate = wavParams[2]
        # print("nsampRate:%d" % nsampRate)
        txt.write('//Sample Rate:%dHz\n'% (nsampRate))

        perSampleData = nChannels * nSampwidth *nsampRate
        print("perSampleData:%f"%(perSampleData))
        playTime = self.wavDataLength()/perSampleData
        print("playTime:%.2fs" % (playTime))
        txt.write('//play time:%.2f s\n'%(playTime))

        txt.write('//Note: There is no WAV file header data here.\n')

        txt.write('\nint frame_count = {};\n'.format(self.wavDataLength()))
        txt.write('const uint8_t sound_buff[{}] =\n'.format(self.wavDataLength()))

        txt.write('{\n')
        bArray = self.byteArray()
        txt.write(self.byteFormatedString(bArray))
        txt.write('\n};\n')
        txt.close()

def convert(fpath):
    wav = wavFile(fpath)
    wavProps = wav.describe()
    print(wavProps)
    print("file size %d bytes"%(wav.wavLength()))
    print('writing file')
    wav.writeFile()
    print('%s convert done'%(fpath))

def listFile():
    filePyth = "./"
    names = ""
    fileNames = os.listdir(filePyth)
    for fileName in fileNames:
        if(fileName.endswith(".wav")):
            print(fileName)
            convert(fileName)
    return names

def main():
    # names = listFile()
    # print(names)
    # # for wavName in enumerate(names):
    # #     print(wavName)
    # fpath = '16s22.wav'
    # wav = wavFile(fpath)
    # wavProps = wav.describe()
    # print(wavProps)
    # print(wav.wavLength())
    # print('writing file')
    # wav.writeFile()
    # print('done')
    print("作者：李支超")
    print("创作时间：2022年8月25日")
    print("简介：该程序将与之同一目录文件夹下的WAV文件转换为对应名称的.h文件十六进制数组")
    listFile()


main()