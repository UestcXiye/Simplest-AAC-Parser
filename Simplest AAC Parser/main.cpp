/**
* ��򵥵� AAC ��Ƶ������������
* Simplest AAC Parser
*
* ԭ����
* ������ Lei Xiaohua
* leixiaohua1020@126.com
* �й���ý��ѧ/���ֵ��Ӽ���
* Communication University of China / Digital TV Technology
* http://blog.csdn.net/leixiaohua1020
*
* �޸ģ�
* ���ĳ� Liu Wenchen
* 812288728@qq.com
* ���ӿƼ���ѧ/������Ϣ
* University of Electronic Science and Technology of China / Electronic and Information Science
* https://blog.csdn.net/ProgramNovice
*
* ����Ŀ��һ�� AAC �����������򣬿��Է��벢���� ADTS ֡��
*
* This project is an AAC stream analysis program.
* It can parse AAC bitstream and analysis ADTS frame of stream.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// �������fopen() ��������ȫ
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
			// һ�� ADTS header ����ռ 7 �ֽڣ����С�� 7 �ֽڣ�˵������һ�� ADTS frame �����ݲ�����
			return -1;
		}
		// ͬ����ռ 12 bit��Ϊ 0xfff
		if ((buffer[0] == 0xFF) && ((buffer[1] & 0xF0) == 0xF0))
		{
			// frame_length��13 bit����ʾ��ǰ ADTS ֡�ĳ��ȣ��洢�ڵ� 4 ���ֽڵĺ���λ���� 5 ���ֽڣ��� 6 ���ֽڵ�ǰ��λ
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

			// ID��1 bit����ʾ MPEG �汾���洢�ڵ� 2 �ֽڵĵ������
			// ֵΪ 0 ��ʾ MPEG-4��ֵΪ 1 ��ʾ MPEG-2
			unsigned char id = (aacframe[1] & 0x08) >> 3;
			// Layer��2 bit����ʾ��Ƶ�������Ĳ㼶���洢�ڵ� 2 �ֽڵĵ� 6��7 ����
			unsigned char layer = (aacframe[1] & 0x00) >> 1;
			// Protection Absent��1 bit��ָʾ�Ƿ����� CRC ����У�飬�洢�ڵ� 2 �ֽڵ����һ������
			// 1 ��ʾû�� CRC������ ADST ͷΪ 7 �ֽڣ�0 ��ʾ�� CRC������ ADST ͷΪ 9 �ֽ�
			unsigned char protection_absent = aacframe[1] & 0x01;
			// profile��2 bit����ʾ AAC ��񣬴洢�ڵ� 3 �ֽڵ�ͷ��λ
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

			// Sampling Frequency Index��4 bit����ʾ�����ʵ��������洢�ڵ� 3 �ֽڵ� 3~6 λ
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

			// Private Bit��1 bit��˽�б��أ��洢�ڵ� 3 �ֽڵĵ��߱���
			unsigned char private_bit = (aacframe[2] & 0x02) >> 1;

			// Channel Configuration��3 bit����ʾ��Ƶ��ͨ�������洢�ڵ� 3 ���ֽڵ����һ���أ��� 4 ���ֽ�ǰ��������
			unsigned char channel_configuration = 0;
			channel_configuration |= ((aacframe[2] & 0x01) << 2); // high 1 bit
			channel_configuration |= ((aacframe[3] & 0xC0) >> 6); // low 2 bit
			// Originality��1 bit���洢�ڵ� 4 ���ֽڵĵ�������
			unsigned char originality = (aacframe[3] & 0x20) >> 5;
			// Home��1 bit���洢�ڵ� 4 ���ֽڵĵ��ı���
			unsigned char home = (aacframe[3] & 0x10) >> 4;
			// Copyright Identification Bit��1 bit���洢�ڵ� 4 ���ֽڵĵ������
			unsigned char copyright_identification_bit = (aacframe[3] & 0x08) >> 3;
			// Copyright Identification Start��1 bit���洢�ڵ� 4 ���ֽڵĵ�������
			unsigned char copyright_identification_start = (aacframe[3] & 0x04) >> 2;
			// Adts Buffer Fullness��11 bit���洢�ڵ� 6 ���ֽڵĺ�������أ��� 7 ���ֽڵ�ǰ��������
			// 0x7FF ��ʾ���ʿɱ��������0x000 ��ʾ�̶����ʵ�����
			int adts_buffer_fullness = 0;
			adts_buffer_fullness |= ((aacframe[5] & 0x1F) << 6); // high 5 bit
			adts_buffer_fullness |= ((aacframe[6] & 0xFC) >> 2); // low 6 bit
			// �洢�ڵ� 7 ���ֽڵ���������أ����ֶα�ʾ��ǰ ADST ֡���������� AAC ֡�ĸ�����һ
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