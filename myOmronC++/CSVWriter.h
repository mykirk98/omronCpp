#pragma once
#include <fstream>
#include <string>
#include <mutex>
#include <cstdint>

class CSVWriter {
public:
    explicit CSVWriter(const std::string& filename)
    {
        open(filename);
    }

    ~CSVWriter()
    {
        close();
    }

    void open(const std::string& filename)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        close();    // 이미 열려 있으면 닫기
        file_.open(filename, std::ios::out | std::ios::trunc);
        if (file_.is_open())
        {
            // 헤더 작성
            file_ << "frameID,before_frame_acq,after_frame_acq,after_save_img,after_cvt_cv\n";
			file_.flush();      // flush() : 버퍼에 있는 내용을 즉시 파일에 기록
        }
    }

    bool is_open() const
    {
        return file_.is_open();
    }

    void close()
    {
        if (file_.is_open())
        {
            file_.close();
        }
    }

    // ns 단위 정수 그대로 기록 (사후 분석에서 ms 변환)
    void WriteRow(uint64_t frameID, long long before_frame_acq, long long after_frame_acq)
    {
		std::lock_guard<std::mutex> lock(mutex_);
        // 파일이 열려 있지 않으면 아무 작업도 수행하지 않음
        if (!file_.is_open())
            return;
        file_ << frameID << ","
            << before_frame_acq << ","
            << after_frame_acq << "\n";
        // 필요 시 주기적으로 flush (대용량일 땐 생략/배치 권장)
        // file_.flush();
	}

	// ns 단위 정수 그대로 기록 (사후 분석에서 ms 변환)
    void WriteRow(uint64_t frameID, long long before_frame_acq, long long after_frame_acq,
                    long long after_save_img, long long after_cvt_cv)
    {
        std::lock_guard<std::mutex> lock(mutex_);
		// 파일이 열려 있지 않으면 아무 작업도 수행하지 않음
        if (!file_.is_open())
            return;
        file_ << frameID << ","
            << before_frame_acq << ","
            << after_frame_acq << ","
            << after_save_img << ","
            << after_cvt_cv << "\n";
        // 필요 시 주기적으로 flush (대용량일 땐 생략/배치 권장)
        // file_.flush();
    }

private:
    std::ofstream file_;
    mutable std::mutex mutex_;
};
