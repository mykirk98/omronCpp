#pragma once
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#endif

/* @brief LCP24100SS ���� ��Ʈ�ѷ� RS-232 ���� Ŭ���� */
class LCP24100SS {
public:
	/* @brief LCP24100SS ������ */
    LCP24100SS();
	/* @brief LCP24100SS �Ҹ��� */
    ~LCP24100SS();

	/* @brief �ø��� ��Ʈ ���� �޼ҵ�
	@param port �ø��� ��Ʈ �̸�
	@param baud ������Ʈ (�⺻��: 19200) */
    bool open(const std::string& port, unsigned long baud = 19200);
	/* @brief �ø��� ��Ʈ �ݱ� �޼ҵ� */
    void close();
	/* @brief �ø��� ��Ʈ ���� ���� */
    bool isOpen() const;

	/* @brief ��� ���� �޼ҵ�
	@param channel ä�� ����
	@param data ��� �� (0~255) */
    bool setBrightness(char channel, int data);
	/* @brief ��Ʈ�κ� �ð� ���� �޼ҵ�
	@param channel ä�� ����
	@param data ��Ʈ�κ� �ð� (0.00~9.99 ms) */
    bool setStrobeTime_ms(char channel, double data);
	/* @brief Ʈ���� ��ȣ �߻� �޼ҵ�
	@param channel ä�� ���� */
    bool trigger(char channel);

private:
#ifdef _WIN32
    void* hSerial_; // HANDLE
#else
    int   fd_;      // file descriptor
#endif // _WIN32
    bool  open_;

	/* @brief �ø��� ��Ʈ�� ��� �����͸� �� ������ �ݺ��ؼ� ���� �Լ�
	@param buf �� ������ ����
	@param len ���� ���� (����Ʈ ����) */
    bool writeAll(const void* buf, unsigned long len);
	/* @brief �ø��� ��Ʈ�� ���ڿ� �����͸� �� ������ �ݺ��ؼ� ���� �Լ�
	@param bytes �� ���ڿ� ������ */
    bool writeAll(const std::string& bytes);
	/* @brief ������ ������
	@param ch ä�� ����
	@param mode1 ���� ���
	@param data3 ������ �� */
    std::string makeFrame(char ch, char mode1, int data3) const;

#ifndef _WIN32
	/* @brief termios ���� �޼ҵ�
	@param baud ������Ʈ */
    bool setupTermios(unsigned long baud);
#endif
};
