
//---------------------------------------------
// DirectXTK for Audio uses XAudio 2
// Ver XAudio2.9
// XAudio2=XAudio + DirectSound
//---------------------------------------------
#include "sound.h"
#include <cwchar>


//BGMテーブル
wchar_t SoundAssetsBGName[(int)SoundBGAssets::SoundBGMax][50] = {
	L"assets/sound/act.wav",
	L"assets/sound/actno.wav",
	
};

//Effect音テーブル
wchar_t SoundAssetsSEName[(int)SoundSEAssets::SoundSEMax][50] = {
	L"assets/sound/hit.wav",
	L"assets/sound/accept.wav",
	L"assets/sound/jump.wav",
	L"assets/sound/shot1.wav",
	L"assets/sound/dm.wav",
	L"assets/sound/dm1.wav",
	L"assets/sound/Towerdm.wav",
	L"assets/sound/cEND.wav",
	L"assets/sound/Reroll.wav",

};

//wchar_t* GetSoundAssetsBGName(int i) {
//
//	return SoundAssetsBGName[i];
//
//}
//wchar_t* GetSoundAssetsSEName(int i) {
//
//	return SoundAssetsSEName[i];
//
//}

void XAudSound::soundInit(){

	DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
	eflags |= DirectX::AudioEngine_Debug;
#endif
	//オーディオエンジンm_audEngine

	m_audEngine = std::make_unique<DirectX::AudioEngine>(eflags);
	//弾発射音ロード
	for (int i = 0; i < (int)SoundSEAssets::SoundSEMax; i++) {
		m_explode[i] = std::make_unique<DirectX::SoundEffect>(m_audEngine.get(),
			SoundAssetsSEName[i]);
	}
	//BGM 背景音楽ロード
	for (int i = 0; i < (int)SoundBGAssets::SoundBGMax; i++) {
		m_ambient[i] =
			std::make_unique<DirectX::SoundEffect>(m_audEngine.get(),
				SoundAssetsBGName[i]);
	}


	std::random_device rd;
	m_random = std::make_unique<std::mt19937>(rd());

}

void XAudSound::soundBGMPlay(int i) {

	m_nightLoop = m_ambient[i]->CreateInstance();
	m_nightLoop->SetVolume(0.4f);
	m_nightLoop->Play(true);


}

void XAudSound::soundSEPlay(int i) {

	float volume = 1.0f;
	float pitch = 0.0f;
		//-0.5f;//周波数
	static float pan = 0.0f; //空間音響
	static float panspeed = 0.001f;
	pan += panspeed;
	if (pan >= 1.0f) {

		pan = -0.1f;
	}

	m_explode[i]->Play(volume, pitch, pan);

}

void XAudSound::soundSEPlay(int i, float pitch)
{
	float volume = 1.0f;
	float pan = 0.0f;
	m_explode[i]->Play(volume, pitch, pan);
}

void XAudSound::soundBGMStop() {

	m_nightLoop->Stop();
	
}



void XAudSound::soundDispose() {

	//オーディオエンジン停止
	if (m_audEngine)
	{
		if (m_disposed) return;
		m_disposed = true;

		m_audEngine->Suspend();
	}

}

XAudSound* GetXAud()
{
	return XAudSound::GetInstance();
}

