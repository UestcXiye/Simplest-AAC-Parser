/**
* 最简单的 AAC 音频码流解析程序
* Simplest AAC Parser
*
* 原程序：
* 雷霄骅 Lei Xiaohua
* leixiaohua1020@126.com
* 中国传媒大学/数字电视技术
* Communication University of China / Digital TV Technology
* http://blog.csdn.net/leixiaohua1020
*
* 修改：
* 刘文晨 Liu Wenchen
* 812288728@qq.com
* 电子科技大学/电子信息
* University of Electronic Science and Technology of China / Electronic and Information Science
* https://blog.csdn.net/ProgramNovice
*
* 本项目是一个 AAC 码流分析程序，可以分离并解析 ADTS 帧。
*
* This project is an AAC stream analysis program.
* It can parse AAC bitstream and analysis ADTS frame of stream.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 解决报错：fopen() 函数不安全
#pragma warning(disable:4996)

int getADTSframe(unsigned char* buffer, int buf_size, unsigned char* data, int* data_size)
{
	int size = 0;

	if (!buffer || !data || !data_size)
	{
		return -1;
	}

	while (1)
	{
		if (buf_size < 7)
		{
			// 一个 ADTS header 最少占 7 字节，如果小于 7 字节，说明不是一个 ADTS frame 或数据不完整
			return -1;
		}
		// 同步字占 12 bit，为 0xfff
		if ((buffer[0] == 0xFF) && ((buffer[1] & 0xF0) == 0xF0))
		{
			// frame_length，13 bit，表示当前 ADTS 帧的长度，存储在第 4 个字节的后两位，第 5 个字节，第 6 个字节的前三位
			size |= ((buffer[3] & 0x03) << 11); // high 2 bit
			size |= buffer[4] << 3; // middle 8 bit
			size |= ((buffer[5] & 0xE0) >> 5); // low 3 bit
			break;
		}
		--buf_size;
		++buffer;
	}

	if (buf_size < size) {
		return 1;
	}

	memcpy(data, buffer, size);
	*data_size = size;

	return 0;
}

int simplest_aac_parser(char *url)
{
	int data_size = 0;
	int size = 0;
	int frame_cnt = 0;
	int offset = 0;

	// FILE *myout = fopen("output_log.txt", "wb+");
	FILE *myout = stdout;

	unsigned char *aacframe = (unsigned char *)malloc(1024 * 5);
	unsigned char *aacbuffer = (unsigned char *)malloc(1024 * 1024);

	FILE *ifile = fopen(url, "rb");
	if (!ifile)
	{
		printf("Open file error.\n");
		return -1;
	}

	printf("-----+----+-------+-------------------+- ADTS Frame Table --+---------+------+-----------------+--------+\n");
	printf(" NUM | ID | Layer | Protection Absent | Profile | Frequency | Channel | Size | Buffer Fullness | Blocks |\n");
	printf("-----+----+-------+-------------------+---------------------+---------+------+-----------------+--------+\n");

	while (!feof(ifile))
	{
		data_size = fread(aacbuffer + offset, 1, 1024 * 1024 - offset, ifile);
		unsigned char* input_data = aacbuffer;

		while (1)
		{
			int ret = getADTSframe(input_data, data_size, aacframe, &size);
			if (ret == -1)
				break;
			else if (ret == 1)
			{
				memcpy(aacbuffer, input_data, data_size);
				offset = data_size;
				break;
			}

			char profile_str[50] = { 0 };
			char frequence_str[10] = { 0 };

			// ID，1 bit，表示 MPEG 版本，存储在第 2 字节的第五比特
			// 值为 0 表示 MPEG-4，值为 1 表示 MPEG-2
			unsigned char id = (aacframe[1] & 0x08) >> 3;
			// Layer，2 bit，表示音频流所属的层级，存储在第 2 字节的第 6、7 比特
			unsigned char layer = (aacframe[1] & 0x00) >> 1;
			// Protection Absent，1 bit，指示是否启用 CRC 错误校验，存储在第 2 字节的最后一个比特
			// 1 表示没有 CRC，整个 ADST 头为 7 字节；0 表示有 CRC，整个 ADST 头为 9 字节
			unsigned char protection_absent = aacframe[1] & 0x01;
			// profile，2 bit，表示 AAC 规格，存储在第 3 字节的头两位
			unsigned char profile = (aacframe[2] & 0xC0) >> 6;
			if (id == 1) // MPEG-2
			{
				switch (profile)
				{
				case 0:
					sprintf(profile_str, "Main");
					break;
				case 1:
					sprintf(profile_str, "LC");
					break;
				case 2:
					sprintf(profile_str, "SSR");
					break;
				default:
					sprintf(profile_str, "unknown");
					break;
				}
			}
			else // MPEG-4
			{
				switch (profile)
				{
				case 0: sprintf(profile_str, "AAC MAIN"); break;
				case 1: sprintf(profile_str, "AAC LC"); break;
				case 2: sprintf(profile_str, "AAC SSR"); break;
				case 3: sprintf(profile_str, "AAC LTP"); break;
				case 4: sprintf(profile_str, "SBR"); break;
				case 5: sprintf(profile_str, "AAC scalable"); break;
				case 6: sprintf(profile_str, "TwinVQ"); break;
				case 7: sprintf(profile_str, "CELP"); break;
				case 8: sprintf(profile_str, "HVXC"); break;
				case 11: sprintf(profile_str, "TTSI"); break;
				case 12: sprintf(profile_str, "Main synthetic"); break;
				case 13: sprintf(profile_str, "Wavetable synthesis"); break;
				case 14: sprintf(profile_str, "General MIDI"); break;
				case 15: sprintf(profile_str, "Algorithmic Synthesis and Audio FX"); break;
				default:
					sprintf(profile_str, "reversed");
					break;
				}
			}

			// Sampling Frequency Index，4 bit，表示采样率的索引，存储在第 3 字节的 3~6 位
			unsigned char sampling_frequency_index = (aacframe[2] & 0x3C) >> 2;
			switch (sampling_frequency_index)
			{
			case 0: sprintf(frequence_str, "96000Hz"); break;
			case 1: sprintf(frequence_str, "88200Hz"); break;
			case 2: sprintf(frequence_str, "64000Hz"); break;
			case 3: sprintf(frequence_str, "48000Hz"); break;
			case 4: sprintf(frequence_str, "44100Hz"); break;
			case 5: sprintf(frequence_str, "32000Hz"); break;
			case 6: sprintf(frequence_str, "24000Hz"); break;
			case 7: sprintf(frequence_str, "22050Hz"); break;
			case 8: sprintf(frequence_str, "16000Hz"); break;
			case 9: sprintf(frequence_str, "12000Hz"); break;
			case 10: sprintf(frequence_str, "11025Hz"); break;
			case 11: sprintf(frequence_str, "8000Hz"); break;
			case 12: sprintf(frequence_str, "7350Hz"); break;
			case 13:
			case 14:
				sprintf(frequence_str, "reversed"); break;
			case 15: sprintf(frequence_str, "escape value"); break;
			default:
				sprintf(frequence_str, "unknown"); break;
			}

			// Private Bit，1 bit，私有比特，存储在第 3 字节的第七比特
			unsigned char private_bit = (aacframe[2] & 0x02) >> 1;

			// Channel Configuration，3 bit，表示音频的通道数，存储在第 3 个字节的最后一比特，第 4 个字节前两个比特
			unsigned char channel_configuration = 0;
			channel_configuration |= ((aacframe[2] & 0x01) << 2); // high 1 bit
			channel_configuration |= ((aacframe[3] & 0xC0) >> 6); // low 2 bit
			// Originality，1 bit，存储在第 4 个字节的第三比特
			unsigned char originality = (aacframe[3] & 0x20) >> 5;
			// Home，1 bit，存储在第 4 个字节的第四比特
			unsigned char home = (aacframe[3] & 0x10) >> 4;
			// Copyright Identification Bit，1 bit，存储在第 4 个字节的第五比特
			unsigned char copyright_identification_bit = (aacframe[3] & 0x08) >> 3;
			// Copyright Identification Start，1 bit，存储在第 4 个字节的第六比特
			unsigned char copyright_identification_start = (aacframe[3] & 0x04) >> 2;
			// Adts Buffer Fullness，11 bit，存储在第 6 个字节的后五个比特，第 7 个字节的前六个比特
			// 0x7FF 表示码率可变的码流，0x000 表示固定码率的码流
			int adts_buffer_fullness = 0;
			adts_buffer_fullness |= ((aacframe[5] & 0x1F) << 6); // high 5 bit
			adts_buffer_fullness |= ((aacframe[6] & 0xFC) >> 2); // low 6 bit
			// 存储在第 7 个字节的最后两比特，该字段表示当前 ADST 帧中所包含的 AAC 帧的个数减一
			unsigned char number_of_raw_data_blocks_in_frame = aacframe[6] & 0x03;

			// NUM | ID | Layer | Protection Absent | Profile | Frequency | Channel | Size | Buffer Fullness | Blocks
			fprintf(myout, "%5d|%4d|%7d|%19d|%9s|%11s|%9d|%6d|            %#X|%8d|\n",
				frame_cnt, id, layer, protection_absent, profile_str, frequence_str, channel_configuration,
				size, adts_buffer_fullness, number_of_raw_data_blocks_in_frame);
			data_size -= size;
			input_data += size;
			frame_cnt++;
		}
	}

	fclose(ifile);
	free(aacbuffer);
	free(aacframe);

	return 0;
}

int main()
{
	char in_filename[] = "tdjm.aac";
	simplest_aac_parser(in_filename);

	system("pause");
	return 0;
}