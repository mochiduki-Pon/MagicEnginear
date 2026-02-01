#pragma once
//---------------------------------------------
// DirectXTK for Audio uses XAudio 2
// 
// NuGet package manager
// If you used NuGet when Adding the DirectX Tool Kit, 
// then you already have support for DirectX Tool Kit for Audio..
// directxtk_desktop_win10
// These packages are configured for Windows 10 
// and Windows 11, 
// and makes use of XAudio 2.9 for DirectX Tool Kit for Audio.
//---------------------------------------------
#include <iostream>

#include <random>
//
// pch.h
// Header for standard system include files.
//

// Make sure math.h defines M_PI
#define _USE_MATH_DEFINES

#include "Audio.h"

enum SoundBGAssets {
	Title1,
	Stage1,
	SoundBGMax,
};

enum SoundSEAssets {
	Search,
	SeAccept,
	Sejump,
	shot1,
	SeDmg,
	SeDmg1,
	Towerdm,
	cEND,
	change,
	SoundSEMax,
};


//Audio Engine
//Assets BGM,Effect音
class XAudSound
{

//XAudio 2.9
//DirectX Tool Kit Audio
//use NuGet package
//The basic game loop with the audio engine
private:
//オーディオエンジン
	std::unique_ptr<DirectX::AudioEngine> m_audEngine;
	//弾発射音、背景音楽
	std::unique_ptr<DirectX::SoundEffect> m_explode[SoundSEMax];
	std::unique_ptr<DirectX::SoundEffect> m_ambient[SoundBGMax];
	//遅延用
	//float explodeDelay = 0.0f;

	std::unique_ptr<std::mt19937> m_random;

	//Loop Music インスタンス
	std::unique_ptr<DirectX::SoundEffectInstance> m_nightLoop;

	bool m_disposed = false;

public:
	static XAudSound* GetInstance() {

		static XAudSound audsound;

		return &audsound;
	}
	 void soundInit();
	 void soundBGMPlay(int i);
	 void soundSEPlay(int i);
	 void soundSEPlay(int i, float pitch);
	 void soundBGMStop();
	 void soundDispose();
	
	
};

XAudSound* GetXAud();



