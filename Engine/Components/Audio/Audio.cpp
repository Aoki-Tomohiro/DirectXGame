#include "Audio.h"
#include <cassert>

Audio* Audio::instance = nullptr;

Audio* Audio::GetInstance() {
	if (instance == nullptr) {
		instance = new Audio();
	}
	return instance;
}

void Audio::DeleteInstance() {
	delete instance;
	instance = nullptr;
}

void Audio::Finalize() {
	//ボイスデータ開放
	for (int i = 0; i < sourceVoices_.size(); i++) {
		if (sourceVoices_[i] != nullptr) {
			sourceVoices_[i]->DestroyVoice();
		}
	}
	//XAudio2解放
	xAudio2_.Reset();
	//音声データ開放
	for (int i = 0; i < soundDatas_.size(); i++) {
		SoundUnload(&soundDatas_[i]);
	}
}

void Audio::Initialize() {
	HRESULT result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	result = xAudio2_->CreateMasteringVoice(&masterVoice_);
}

uint32_t Audio::SoundLoadWave(const char* filename) {
	audioHandle_++;
	//ファイル入力ストリームのインスタンス
	std::ifstream file;
	//.wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	//ファイルオープン失敗を検出する
	assert(file.is_open());


	//RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	//ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	//タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	//Formatチャンクの読み込み
	FormatChunk format = {};
	//チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	//チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	//Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	//JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		//読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		//再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	//Dataチャンクのデータ部(波形データ)の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	//Waveファイルを閉じる
	file.close();


	//returnする為の音声データ
	soundDatas_[audioHandle_].wfex = format.fmt;
	soundDatas_[audioHandle_].pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundDatas_[audioHandle_].bufferSize = data.size;

	return audioHandle_;
}

void Audio::SoundUnload(SoundData* soundData) {
	//バッファのメモリを解放
	delete[] soundData->pBuffer;
	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

//void Audio::SoundPlayWave(uint32_t audioHandle, bool roopFlag) {
//	HRESULT result;
//	//波形フォーマットを元にSourceViceの作成
//	IXAudio2SourceVoice* pSourceVoice = nullptr;
//	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundDatas_[audioHandle].wfex);
//	assert(SUCCEEDED(result));
//
//	//再生する波形データの設定
//	XAUDIO2_BUFFER buf{};
//	buf.pAudioData = soundDatas_[audioHandle].pBuffer;
//	buf.AudioBytes = soundDatas_[audioHandle].bufferSize;
//	buf.Flags = XAUDIO2_END_OF_STREAM;
//	if (roopFlag) {
//		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
//	}
//
//	//波形データの再生
//	result = pSourceVoice->SubmitSourceBuffer(&buf);
//	result = pSourceVoice->Start();
//}

void Audio::SoundPlayWave(uint32_t audioHandle, bool roopFlag) {
	HRESULT result;
	//波形フォーマットを元にSourceVoiceの作成
	sourceVoices_[audioHandle] = nullptr;
	result = xAudio2_->CreateSourceVoice(&sourceVoices_[audioHandle], &soundDatas_[audioHandle].wfex);
	assert(SUCCEEDED(result));

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundDatas_[audioHandle].pBuffer;
	buf.AudioBytes = soundDatas_[audioHandle].bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	if (roopFlag) {
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	//波形データの再生
	result = sourceVoices_[audioHandle]->SubmitSourceBuffer(&buf);
	result = sourceVoices_[audioHandle]->Start();
}

void Audio::StopAudio(uint32_t audioHandle) {
	HRESULT result;
	result = sourceVoices_[audioHandle]->Stop();
}