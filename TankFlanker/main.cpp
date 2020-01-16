﻿#define NOMINMAX
#include "define.h"
#include "useful.h"
//#include <algorithm>
//#include <memory>
/*main*/
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	/*変数*/
	int mousex, mousey;	  /*mouse*/
	std::array<bool, 20> keyget; /*キー用*/
	std::array<bool, 4> keyget2; /*キー用*/
	bool out{ false };	   /*終了フラグ*/
	std::vector<pair> pssort;    /*playerソート*/
	std::vector<players> player; /*player*/
	VECTOR_ref aimpos;	   /*照準器座標確保用*/
	float aimdist{ 0.f };	/*照準距離確保用*/
	uint8_t selfammo[2];	 /*弾種変更キー*/
	switches aim, map;	   /*視点変更*/
	float ratio, rat_r, aim_r;   /*カメラ倍率、実倍率、距離*/
	float rat_aim;		     /*照準視点倍率*/
	size_t waysel, choose;       /*指揮視点　指揮車両、マウス選択*/
	uint8_t way[2];		     /*マウストリガー*/
	float frate;		     /*基準フレームレート*/
	float fps;		     /*fps*/
	LONGLONG old_time, waits;    /*時間取得*/
	VECTOR_ref cam, view, upvec; /*カメラ*/
	int mapc;
	//init----------------------------------------------------------------------------------//
	auto parts = std::make_unique<Myclass>();							       /*汎用クラス*/
	auto humanparts = std::make_unique<HUMANS>(parts->get_usegrab(), parts->get_f_rate());		       /*車内関係*/
	auto mapparts = std::make_unique<MAPS>(parts->get_gndx(), parts->get_drawdist(), parts->get_shadex()); /*地形、ステージ関係*/
	auto uiparts = std::make_unique<UIS>();
	frate = parts->get_f_rate();
	//
	//parts->autoset_option();//オプション自動セット
	//parts->write_option(); //オプション書き込み
	//load----------------------------------------------------------------------------------//
	parts->set_fonts(18);
	SetUseASyncLoadFlag(TRUE);
	//hit---------------------------------------------------------------------------//
	const auto hit_mod = MV1ModelHandle::Load("data/hit/hit.mv1");
	//screen------------------------------------------------------------------------//
	int minimap = MakeScreen(dispx, dispy, FALSE);			     /*ミニマップ*/
	int skyscreen = MakeScreen(dispx, dispy, FALSE);		     /*空*/
	int mainscreen = MakeScreen(dispx, dispy, FALSE);		     /*遠景*/
	int HighBrightScreen = MakeScreen(dispx, dispy, FALSE);		     /*エフェクト*/
	int GaussScreen = MakeScreen(dispx / EXTEND, dispy / EXTEND, FALSE); /*エフェクト*/
	SetUseASyncLoadFlag(FALSE);
	uiparts->draw_load(); //
	if (!parts->set_veh())
		return -1;
	/*物理開始*/
	auto world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f)); /* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
	//これ以降繰り返しロード----------------------------------------------------------------//
	do {
		std::string stage = "data_0";
		{
			const auto mdata = FileRead_open(("stage/" + stage + "/main.txt").c_str(), FALSE); /*ステージ情報*/
			char mstr[64];									   /*tank*/
			FileRead_gets(mstr, 64, mdata);
			mapc = std::stoi(getright(mstr));
			FileRead_close(mdata);
		}
		const size_t teamc = count_team(stage);   /*味方数*/
		const size_t enemyc = count_enemy(stage); /*敵数*/
		player.resize(teamc + enemyc);
		pssort.resize(teamc + enemyc);
		for (int p_cnt = 0; p_cnt < teamc + enemyc; ++p_cnt)
			player[p_cnt].id = p_cnt;
		{
			const int m = parts->window_choosev(); /*player指定*/
			if (m == -1)
				return 0;
			//設定
			for (auto&& p : player) {
				if (p.id < teamc) {
					p.type = TEAM;
					{
						const auto mdata = FileRead_open(("stage/" + stage + "/team/" + std::to_string(p.id) + ".txt").c_str(), FALSE);
						//char mstr[64]; /*tank*/
						p.use = (p.id == 0) ? m : 1; //2;
						p.pos = VGet(20.0f * p.id, 0.0f, -400.0f);
						FileRead_close(mdata);
					}
				}
				else {
					p.type = ENEMY;
					{
						const auto mdata = FileRead_open(("stage/" + stage + "/enemy/" + std::to_string(p.id - teamc) + ".txt").c_str(), FALSE);
						//char mstr[64]; /*tank*/
						p.use = 0; //1; // p.id % parts->get_vehc();
						p.pos = VGet(20.0f * (p.id - teamc), 0.0f, 400.0f);
						FileRead_close(mdata);
					}
				}
			}
		}
		for (auto&& p : player) {
			p.yrad = DX_PI_F * p.type;
			std::fill(std::begin(p.waypos), std::end(p.waypos), p.pos);
			std::fill(std::begin(p.wayspd), std::end(p.wayspd), 2);
			/*vehsから引き継ぎ*/
			p.ptr = parts->get_vehicle(p.use);
		}
		/*UI*/
		uiparts->set_state(&player[0]);
		/*load*/
		SetUseASyncLoadFlag(TRUE);
		SetCreate3DSoundFlag(TRUE);
		for (auto&& p : player) {
			p.obj = p.ptr->model.Duplicate();
			p.colobj = p.ptr->colmodel.Duplicate();
			for (auto&& h : p.hit)
				h.pic = hit_mod.Duplicate();
			size_t i = 0;
			p.se.emplace_back(SoundHandle::Load("data/audio/se/engine/0.wav"));
			i++;
			p.se.emplace_back(SoundHandle::Load("data/audio/se/fire/gun.wav"));
			i++;
			for (; i < 10; ++i)
				p.se.emplace_back(SoundHandle::Load("data/audio/se/fire/" + std::to_string(i - 2) + ".wav"));
			for (; i < 27; ++i)
				p.se.emplace_back(SoundHandle::Load("data/audio/se/ricochet/" + std::to_string(i - 10) + ".wav"));
			for (; i < 29; ++i)
				p.se.emplace_back(SoundHandle::Load("data/audio/se/engine/o" + std::to_string(i - 27) + ".wav"));
			for (; i < 31; ++i)
				p.se.emplace_back(SoundHandle::Load("data/audio/se/hit_enemy/" + std::to_string(i - 29) + ".wav"));

			p.se.emplace_back(SoundHandle::Load("data/audio/se/turret/" + std::to_string(0) + ".wav"));
			i++;
		}
		SetCreate3DSoundFlag(FALSE);
		SetUseASyncLoadFlag(FALSE);
		mapparts->set_map_readyb(mapc);
		/*human*/
		if (!humanparts->set_humans(player[0].ptr->inmodel))
			return 0;
		uiparts->draw_load(); //
		/*map*/
		if (!mapparts->set_map_ready())
			break;
		//players
		const auto c_ffff96 = GetColor(255, 255, 150);
		const auto c_ffc896 = GetColor(255, 200, 150);
		for (auto&& p : player) {
			//色調
			for (int i = 0; i < p.obj.material_num(); ++i) {
				MV1SetMaterialSpcColor(p.obj.get(), i, GetColorF(0.85f, 0.82f, 0.78f, 0.5f));
				MV1SetMaterialSpcPower(p.obj.get(), i, 5.0f);
			}
			MV1SetMaterialDrawAlphaTestAll(p.obj.get(), TRUE, DX_CMP_GREATER, 128);
			//リセット
			p.gear = 0;
			//cpu
			p.atkf.reset();
			p.aim = -1;
			//hit
			for (int i = 0; i < p.ptr->colmodel.mesh_num(); ++i)
				MV1SetupCollInfo(p.colobj.get(), -1, 5, 5, 5, i);
			p.hitssort.resize(p.ptr->colmodel.mesh_num());
			p.hitbuf = 0;
			for (int i = 0; i < p.hit.size(); ++i)
				p.hit[i].flug = false;
			//ammo
			for (size_t guns = 0; guns < gunc; ++guns) {
				p.Gun[guns].Ammo.resize(ammoc);
				for (auto& c : p.Gun[guns].Ammo)
					c.color = (p.type == TEAM) ? c_ffff96 : c_ffc896;
			}
			//HP
			p.HP.resize(p.ptr->colmodel.mesh_num());
			/*0123は装甲部分なので詰め込む*/
			p.HP[0] = 1; //life
			for (size_t i = 4; i < p.HP.size(); ++i) {
				p.HP[i] = 100;
			} //spaceARMER
			//wheel
			p.Springs.resize(p.obj.frame_num());
			//0初期化いる
			//
			MV1SetMatrix(p.colobj.get(), MGetTranslate(VGet(0, 0, 0)));
			//装てん
			p.Gun[0].loadcnt = 1;
			//ypos反映
			{
				auto HitPoly = mapparts->get_gnd_hit(p.pos + VGet(0.0f, (float)map_x, 0.0f), p.pos + VGet(0.0f, -(float)map_x, 0.0f));
				if (HitPoly.HitFlag)
					p.pos = VGet(p.pos.x(), HitPoly.HitPosition.y, p.pos.z());
				for (auto&& w : p.waypos) {
					HitPoly = mapparts->get_gnd_hit(w + VGet(0.0f, (float)map_x, 0.0f), w + VGet(0.0f, -(float)map_x, 0.0f));
					if (HitPoly.HitFlag)
						w = HitPoly.HitPosition;
				}
			}
			//
			p.hitres.resize(p.ptr->colmodel.mesh_num());
		}
		//物理set
		for (auto&& p : player) {
			b2PolygonShape dynamicBox; /*ダイナミックボディに別のボックスシェイプを定義します。*/
			dynamicBox.SetAsBox(
			    (p.ptr->min.x() - p.ptr->max.x()) / 2,
			    (p.ptr->min.z() - p.ptr->max.z()) / 2,
			    b2Vec2(
				(p.ptr->max.x() + p.ptr->min.x()) / 2,
				(p.ptr->max.z() + p.ptr->min.z()) / 2),
			    0.f);					  /**/
			b2FixtureDef fixtureDef;			  /*動的ボディフィクスチャを定義します*/
			fixtureDef.shape = &dynamicBox;			  /**/
			fixtureDef.density = 1.0f;			  /*ボックス密度をゼロ以外に設定すると、動的になります*/
			fixtureDef.friction = 0.3f;			  /*デフォルトの摩擦をオーバーライドします*/
			b2BodyDef bodyDef;				  /*ダイナミックボディを定義します。その位置を設定し、ボディファクトリを呼び出します*/
			bodyDef.type = b2_dynamicBody;			  /**/
			bodyDef.position.Set(p.pos.x(), p.pos.z());       /**/
			bodyDef.angle = p.yrad;				  /**/
			p.body.reset(world->CreateBody(&bodyDef));	/**/
			p.playerfix = p.body->CreateFixture(&fixtureDef); /*シェイプをボディに追加します*/
			/* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
			for (size_t i = 0; i < 2; ++i)
				p.foot[i].LR = (i == 0) ? 1.f : -1.f;

			for (auto& f : p.foot) {
				f.world = new b2World(b2Vec2(0.0f, 0.0f));
				{
					b2Body* ground = NULL;
					{
						b2BodyDef bd;
						ground = f.world->CreateBody(&bd);
						b2EdgeShape shape;
						shape.Set(b2Vec2(-40.0f, -10.0f), b2Vec2(40.0f, -10.0f));
						ground->CreateFixture(&shape, 0.0f);
					}
					b2Body* prevBody = ground;
					size_t i = 0;
					VECTOR_ref vects;
					for (auto& w : p.ptr->upsizeframe) {
						vects = VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w));
						if (vects.x() * f.LR > 0) {
							f.Foot.resize(i + 1);
							b2PolygonShape f_dynamicBox; /*ダイナミックボディに別のボックスシェイプを定義します。*/
							f_dynamicBox.SetAsBox(0.1f, 0.125f);
							b2FixtureDef f_fixtureDef;
							f_fixtureDef.shape = &f_dynamicBox;
							f_fixtureDef.density = 20.0f;
							f_fixtureDef.friction = 0.2f;
							b2BodyDef f_bodyDef;
							f_bodyDef.type = b2_dynamicBody;
							f_bodyDef.position.Set(vects.z(), vects.y());
							f.Foot[i].body.reset(f.world->CreateBody(&f_bodyDef));
							f.Foot[i].playerfix = f.Foot[i].body->CreateFixture(&f_fixtureDef); // シェイプをボディに追加します。
							f.f_jointDef.Initialize(prevBody, f.Foot[i].body.get(), b2Vec2(vects.z(), vects.y()));
							f.world->CreateJoint(&f.f_jointDef);
							prevBody = f.Foot[i].body.get();
							++i;
						}
					}
					f.f_jointDef.Initialize(prevBody, ground, b2Vec2(vects.z(), vects.y()));
					f.world->CreateJoint(&f.f_jointDef);
					i = 0;
					for (auto& w : p.ptr->wheelframe) {
						vects = VECTOR_ref(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w + 1))) +
							VECTOR_ref(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w)));
						if (vects.x() * f.LR > 0) {
							f.Wheel.resize(i + 1);
							b2CircleShape shape;
							shape.m_radius = VSize(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w + 1))) - 0.1f;
							b2FixtureDef fw_fixtureDef;
							fw_fixtureDef.shape = &shape;
							fw_fixtureDef.density = 1.0f;
							b2BodyDef fw_bodyDef;
							fw_bodyDef.type = b2_kinematicBody;
							fw_bodyDef.position.Set(vects.z(), vects.y());
							f.Wheel[i].body.reset(f.world->CreateBody(&fw_bodyDef));
							f.Wheel[i].playerfix = f.Wheel[i].body->CreateFixture(&fw_fixtureDef);
							++i;
						}
					}
					i = 0;
					for (auto& w : p.ptr->youdoframe) {
						vects = VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w));
						if (vects.x() * f.LR > 0) {
							f.Yudo.resize(i + 1);
							b2CircleShape shape;
							shape.m_radius = 0.1f;
							b2FixtureDef fy_fixtureDef;
							fy_fixtureDef.shape = &shape;
							fy_fixtureDef.density = 1.0f;
							b2BodyDef fy_bodyDef;
							fy_bodyDef.type = b2_kinematicBody;
							fy_bodyDef.position.Set(vects.z(), vects.y());
							f.Yudo[i].body.reset(f.world->CreateBody(&fy_bodyDef));
							f.Yudo[i].playerfix = f.Yudo[i].body->CreateFixture(&fy_fixtureDef);
							++i;
						}
					}
				}
			}
		}
		/*音量調整*/
		humanparts->set_humanvc_vol(unsigned char(255.f * parts->get_se_vol()));
		parts->set_se_vol(unsigned char(128.f * parts->get_se_vol()));
		for (auto&& p : player)
			for (auto& s : p.se)
				ChangeVolumeSoundMem(128, s.get());


		const auto c_000000 = GetColor(0, 0, 0);
		const auto c_00ff00 = GetColor(0, 255, 0);
		const auto c_ff0000 = GetColor(255, 0, 0);
		const auto c_008000 = GetColor(0, 128, 0);
		const auto c_800000 = GetColor(128, 0, 0);
		const auto c_ffff00 = GetColor(255, 255, 0);
		const auto c_c8c800 = GetColor(200, 200, 0);
		const auto c_c0ff00 = GetColor(192, 255, 0);
		const auto c_808080 = GetColor(128, 128, 128);
		const auto c_ffffff = GetColor(255, 255, 255);
		const auto c_3232ff = GetColor(50, 50, 255);
		/*メインループ*/
		aim.flug = false; /*照準*/
		map.flug = false; /*マップ*/
		rat_aim = 3.f;    /*照準視点　倍率*/
		ratio = 1.0f;     /*カメラ　　倍率*/
		rat_r = ratio;    /*カメラ　　実倍率*/
		aim_r = 100.0f;   /*照準視点　距離*/
		waysel = 1;       /*指揮視点　指揮車両*/
		parts->set_viewrad(VGet(0.f, player[0].yrad, 1.f));
		SetCursorPos(x_r(960), y_r(540));
		//
		for (auto&& p : player) {
			p.ps_r = MMult(
			    MGetRotY(-p.yrad),
			    MGetRotVec2(VGet(0, 1.f, 0), p.nor.get())); //MMult(MGetRotX(p.xnor), MGetRotZ(p.znor))
			p.zvec = VTransform(VGet(0, 0, -1.f), p.ps_r);
			p.ps_m = MMult(p.ps_r, p.pos.Mtrans());
		}
		//
		PlaySoundMem(player[0].se[31].get(), DX_PLAYTYPE_LOOP, TRUE);
		for (auto& p : player) {
			p.gunrad_rec = VGet(0, 0, 0);
			p.effcs[ef_smoke2].handle = parts->get_effHandle(ef_smoke2).Play3D();
			p.effcs[ef_smoke3].handle = parts->get_effHandle(ef_smoke2).Play3D();
			PlaySoundMem(p.se[0].get(), DX_PLAYTYPE_LOOP, TRUE);
			PlaySoundMem(p.se[27].get(), DX_PLAYTYPE_LOOP, TRUE);
			PlaySoundMem(p.se[28].get(), DX_PLAYTYPE_LOOP, TRUE);
			size_t i = 0;
			for (; i < 2; ++i)
				Set3DRadiusSoundMem(50.0f, p.se[i].get());
			for (; i < 10; ++i)
				Set3DRadiusSoundMem(300.0f, p.se[i].get());
			for (; i < 27; ++i)
				Set3DRadiusSoundMem(100.0f, p.se[i].get());
			for (; i < 29; ++i)
				Set3DRadiusSoundMem(50.0f, p.se[i].get());
			for (; i < 31; ++i)
				Set3DRadiusSoundMem(300.0f, p.se[i].get());
			for (; i < 32; ++i)
				Set3DRadiusSoundMem(10.0f, p.se[i].get());
		}
		humanparts->start_humanvoice(1);

		choose = (std::numeric_limits<size_t>::max)();
		old_time = GetNowHiPerformanceCount() - (LONGLONG)(1000000.0f / frate);
		while (ProcessMessage() == 0) {
			/*fps*/
			waits = GetNowHiPerformanceCount();
			fps = 1000000.0f / (float)(waits - old_time);
			old_time = GetNowHiPerformanceCount();
			uiparts->put_way(); //debug
			//メインスレッド依存
			if (GetActiveFlag() == TRUE) {
				SetMouseDispFlag(FALSE);
				keyget2[0] = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
				keyget2[1] = (GetMouseInput() & MOUSE_INPUT_RIGHT) != 0;
				keyget2[2] = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
				keyget2[3] = CheckHitKey(KEY_INPUT_P) != 0;
				if (player[0].HP[0] > 0) {
					keyget[1] = CheckHitKey(KEY_INPUT_RSHIFT) != 0;

					keyget[2] = CheckHitKey(KEY_INPUT_LSHIFT) != 0;
					keyget[3] = CheckHitKey(KEY_INPUT_V) != 0;
					keyget[4] = CheckHitKey(KEY_INPUT_C) != 0;
					keyget[5] = CheckHitKey(KEY_INPUT_X) != 0;
					keyget[6] = CheckHitKey(KEY_INPUT_Z) != 0;

					keyget[7] = CheckHitKey(KEY_INPUT_Q) != 0;
					keyget[0] = CheckHitKey(KEY_INPUT_E) != 0;

					keyget[9] = CheckHitKey(KEY_INPUT_W) != 0;
					keyget[10] = CheckHitKey(KEY_INPUT_S) != 0;
					keyget[11] = CheckHitKey(KEY_INPUT_A) != 0;
					keyget[12] = CheckHitKey(KEY_INPUT_D) != 0;

					keyget[13] = CheckHitKey(KEY_INPUT_LEFT) != 0;
					keyget[14] = CheckHitKey(KEY_INPUT_RIGHT) != 0;
					keyget[15] = CheckHitKey(KEY_INPUT_UP) != 0;
					keyget[16] = CheckHitKey(KEY_INPUT_DOWN) != 0;
					keyget[8] = CheckHitKey(KEY_INPUT_LCONTROL) != 0;

					keyget[17] = CheckHitKey(KEY_INPUT_SPACE) != 0;
					keyget[18] = CheckHitKey(KEY_INPUT_B) != 0;

					keyget[19] = CheckHitKey(KEY_INPUT_RCONTROL) != 0;
					/*変速*/
				}
				/*指揮*/
				if (map.flug)
					SetMouseDispFlag(TRUE);
			}
			else {
				SetMouseDispFlag(TRUE);
			}
			//依存しない
			if (GetActiveFlag() == TRUE) {
				if (keyget2[2]) {
					out = true;
					break;
				} /*終了*/
				if (keyget2[3])
					break; /*リスタート*/
				if (player[0].HP[0] == 0) {
					for (auto& tt : keyget)
						tt = false;
				}
				/*指揮*/
				if (keyget[1]) {
					map.cnt = std::min<uint8_t>(map.cnt + 1, 2);
					if (map.cnt == 1) {
						map.flug ^= 1;
						SetCursorPos(x_r(960), y_r(540));
					}
				}
				else
					map.cnt = 0;
				/*照準*/
				if (keyget[2]) {
					aim.cnt = std::min<uint8_t>(aim.cnt + 1, 2);
					if (aim.cnt == 1) {
						aim.flug ^= 1;
						//一回だけ進めたいものはここに
						if (aim.flug)
							ratio = rat_aim;
						else {
							rat_aim = ratio;
							ratio = 1.0f;
						}
						map.flug = false;
					}
				}
				else
					aim.cnt = 0;
				/*死んだときは無効*/
				if (player[0].HP[0] == 0) {
					aim.flug = false;
					map.flug = false;
					ratio = 1.0f;
				}
				/*弾種交換*/
				if (keyget[7]) {
					selfammo[0] = std::min<uint8_t>(selfammo[0] + 1, 2);
					if (selfammo[0] == 1) {
						++player[0].ammotype %= 3;
						player[0].Gun[0].loadcnt = 1;
					}
				}
				else if (player[0].Gun[0].loadcnt == 0) {
					selfammo[0] = 0;
				}
				if (keyget[0]) {
					selfammo[1] = std::min<uint8_t>(selfammo[1] + 1, 2);
					if (selfammo[1] == 1) {
						if (--player[0].ammotype == -1)
							player[0].ammotype = 3 - 1;
						player[0].Gun[0].loadcnt = 1;
					}
				}
				else if (player[0].Gun[0].loadcnt == 0) {
					selfammo[1] = 0;
				}
				/*指揮*/
				if (map.flug) {
					GetMousePoint(&mousex, &mousey);
					choose = (std::numeric_limits<size_t>::max)();
					for (auto&& p : player) {
						if (p.id == 0)
							continue;
						if (p.HP[0] > 0)
							if (inm(x_r(132), y_r(162 + p.id * 24), x_r(324), y_r(180 + p.id * 24))) {
								choose = p.id;
								if (keyget2[0])
									waysel = choose;
							}
					}
					if (player[waysel].HP[0] > 0)
						if (inm(x_r(420), y_r(0), x_r(1500), y_r(1080))) {
							if (player[waysel].wayselect <= waypc - 1) {
								if (keyget2[0]) {
									way[0] = std::min<uint8_t>(way[0] + 1, 2);
									if (way[0] == 1) {
										if (player[waysel].wayselect == 0)
											player[waysel].waynow = 0;
										player[waysel].waypos[player[waysel].wayselect] = VGet(_2x(mousex), 0, _2y(mousey));
										for (size_t i = player[waysel].wayselect; i < waypc; ++i)
											player[waysel].waypos[i] = player[waysel].waypos[player[waysel].wayselect];
										++player[waysel].wayselect;
									}
								}
								else
									way[0] = 0;
							}
							if (player[waysel].wayselect > 0) {
								if (keyget2[1]) {
									way[1] = std::min<uint8_t>(way[1] + 1, 2);
									if (way[1] == 1) {
										player[waysel].waynow = std::max<size_t>(player[waysel].waynow - 1, 0);
										--player[waysel].wayselect;
										if (player[waysel].wayselect >= 1)
											for (size_t i = player[waysel].wayselect; i < waypc; ++i)
												player[waysel].waypos[i] = player[waysel].waypos[player[waysel].wayselect - 1];
										else
											for (size_t i = 0; i < waypc; ++i)
												player[waysel].waypos[i] = player[waysel].pos;
									}
								}
								else
									way[1] = 0;
							}
						}
				}
				/*視界見回し*/
				else {
					if (aim.flug) {
						SetCursorPos(x_r(960), y_r(540));
						if (keyget[3])
							ratio = std::min<float>(ratio + 2.0f / frate, 10.f);
						if (keyget[4])
							ratio = std::max<float>(ratio - 2.0f / frate, 2.f);
						if (keyget[5])
							aim_r += 10.0f;
						if (keyget[6])
							aim_r -= 10.0f;
					}
					else
						parts->set_view_r(GetMouseWheelRotVol(),player[0].HP[0]>0);
				}
				differential(rat_r, ratio, 0.1f); /*倍率、測距*/
			}
			//
			if (true) {
				/*操作、座標系*/
				for (auto&& p : player) {
					if (!map.flug)
						p.wayselect = 0;
					if (p.HP[0] > 0) {
						p.move = 0;
						/*操作*/
						if (p.id == 0) {
							for (auto i = 0; i < 10; i++)
								p.move |= keyget[i + 9] << i;
							/*変速*/
							if (set_shift(p))
								parts->play_sound(0);
						}
						/*CPU操作*/
						else {
							/*移動*/
							if ((p.waypos[p.waynow] - p.pos).size() >= 10.0) {
								p.move |= KEY_GOFLONT;
								VECTOR_ref tempv = (p.waypos[p.waynow] - p.pos).Norm();
								if ((p.zvec.z() * tempv.x() + p.zvec.x() * tempv.z()) < 0)
									p.move |= KEY_GOLEFT_;
								else
									p.move |= KEY_GORIGHT;
							}
							else
								p.waynow = std::min<size_t>(p.waynow + 1, waypc - 1);
							//*戦闘
							if (!p.atkf) {
								p.gear = p.wayspd[p.waynow]; //*変速
								for (auto& t : player) {
									if (p.type == t.type || t.HP[0] == 0)
										continue;
									if ((t.pos - p.pos).size() <= 500.0f) //見つける
										if (p.aim != p.atkf) {	//前狙った敵でないか
											p.aim = 0;
											p.atkf = t.id;
											break;
										}
								}
							}
							else {
								{
									auto& t = player[p.atkf.value()];
									VECTOR_ref tempvec[2];
									p.gear = 1; //*変速
									{
										tempvec[1] = p.obj.frame(p.ptr->gunframe[0]);			    //*元のベクトル
										tempvec[0] = (t.obj.frame(t.ptr->gunframe[0]) - tempvec[1]).Norm(); //*向くベクトル

										float tmpf = (t.obj.frame(t.ptr->gunframe[0]) - tempvec[1]).size();
										float tmpf2;

										getdist(tempvec[1], (p.obj.frame(p.ptr->gunframe[0] + 1) - tempvec[1]).Norm(), tmpf, tmpf2, p.ptr->gun_speed[p.ammotype], frate);
										tempvec[1] = (tempvec[1] - p.obj.frame(p.ptr->gunframe[0])).Norm();
									}
									if (cross2D(
										std::hypot(tempvec[0].x(), tempvec[0].z()), tempvec[0].y(),
										std::hypot(tempvec[1].x(), tempvec[1].z()), tempvec[1].y()) <= 0)
										p.move |= KEY_TURNUP_;
									else
										p.move |= KEY_TURNDWN;

									if (cross2D(tempvec[0].x(), tempvec[0].z(), tempvec[1].x(), tempvec[1].z()) < 0) {
										p.move |= KEY_TURNLFT;
										p.move |= KEY_GOLEFT_; //
									}
									else {
										p.move |= KEY_TURNRIT;
										p.move |= KEY_GORIGHT; //
									}
									if ((tempvec[1] * tempvec[0]).size() < sin(deg2rad(1))) {
										const auto HitPoly = mapparts->get_gnd_hit(p.obj.frame(p.ptr->gunframe[0]), t.obj.frame(t.ptr->gunframe[0]));
										if (!HitPoly.HitFlag) {
											if (p.Gun[0].loadcnt == 0) {
												p.move &= ~KEY_GOFLONT;
												p.gear = 0; //*変速
												if (p.speed < 5.f / 3.6f / frate) {
													p.move |= KEY_SHOTCAN;
													p.aim++;
												}
											}
											if (GetRand(100) <= 2)
												p.move |= KEY_SHOTGAN;
										}
									}
									if (t.HP[0] == 0 || p.aim > 5) {
										p.aim = int(p.atkf.value());
										p.atkf = std::nullopt;
									}
								}
							}
							//ぶつかり防止
							for (auto& t : player) {
								if (p.id != t.id && t.HP[0] > 0) {
									if ((t.pos - p.pos).size() <= 10.0) {
										VECTOR_ref tempv = (t.pos - p.pos).Norm();
										if ((p.zvec.z() * tempv.x() + p.zvec.x() * tempv.z()) > 0) {
											p.move |= KEY_GOLEFT_;
											p.move &= ~KEY_GORIGHT;
										}
										else {
											p.move |= KEY_GORIGHT;
											p.move &= ~KEY_GOLEFT_;
										}
									}
								}
								/*
								if (p.state == CPU_NOMAL) {
									if (VSize(VSub(t.pos, p.pos)) <= 250.0 &&  p.id != tgt_p && p.type != t.type &&  p.waynow != waypc - 1) {
										if (p.waynow != 0) {
											p.waynow = waypc - 1;
											for (j = 0; j < waypc; ++j) {
												p.waypos[j] = p.pos;
											}
											player[waysel].wayselect = 0;
										}
										else {
											if (VSize(VSub(p.waypos[p.waynow], t.pos)) > 225.f) {
												p.waynow = waypc - 1;
												p.waypos[p.waynow] = p.pos;
												player[waysel].wayselect = 0;
											}
										}
									}
								}
								*/
							}
						}
					}
					else
						p.move = KEY_TURNUP_;
					if (p.HP[5] == 0 || p.HP[6] == 0) {
						p.move &= ~KEY_GOFLONT;
						p.move &= ~KEY_GOBACK_;
					}
					if (p.HP[5] == 0 && p.HP[6] == 0) {
						p.move &= ~KEY_GOLEFT_;
						p.move &= ~KEY_GORIGHT;
					}
				}
				/*共通動作*/
				for (auto& p : player) {
					if (p.id == 0)
						set_gunrad(p, rat_r * ((keyget[8]) ? 1.f : 3.f));
					else
						set_gunrad(p, 1.f);
					//
					if ((p.move & KEY_GOFLONT) != 0 && p.gear > 0) {
						if (p.gear > 1)
							p.accel = (p.ptr->speed_flont[p.gear - 1] - p.ptr->speed_flont[p.gear - 2]) / (5.0f * frate);
						else if (p.gear == 1)
							p.accel = p.ptr->speed_flont[0] / (5.0f * frate);
						p.speed = std::min<float>(p.speed + p.accel, p.ptr->speed_flont[p.gear - 1]);
					}
					if ((p.move & KEY_GOBACK_) != 0 && p.gear < 0) {
						if (p.gear < 1)
							p.accel = (p.ptr->speed_back[-p.gear - 1] - p.ptr->speed_back[-p.gear - 2]) / (5.0f * frate);
						else if (p.gear == -1)
							p.accel = p.ptr->speed_back[0] / (5.0f * frate);
						p.speed = std::max<float>(p.speed + p.accel, p.ptr->speed_back[-p.gear - 1]);
					}
					//旋回
					{
						float turn_bias = 0.f;
						if (p.HP[5] > 0 || p.HP[6] > 0) {
							turn_bias = 1.0f;
							if ((p.move & KEY_GOFLONT) != 0 && p.gear > 0)
								turn_bias = abs(p.speed / p.ptr->speed_flont[p.gear - 1]);
							if ((p.move & KEY_GOBACK_) != 0 && p.gear < 0)
								turn_bias = abs(p.speed / p.ptr->speed_back[-p.gear - 1]);
							turn_bias *= ((p.HP[5] > 0) + (p.HP[6] > 0)) / 2.0f; //履帯が切れていると
						}
						if (p.yace == 0.0f) {
							if ((p.move & KEY_GOLEFT_) != 0)
								differential(p.yadd, p.ptr->vehicle_RD * turn_bias, 0.1f);
							if ((p.move & KEY_GORIGHT) != 0)
								differential(p.yadd, -p.ptr->vehicle_RD * turn_bias, 0.1f);
						}
					}
					differential(p.inertiax, (p.speed - p.speedrec), 0.02f);
					p.speedrec = p.speed;
					differential(p.inertiaz, -(p.znor - p.znorrec) / 2.0f, 0.1f);
					p.znorrec = p.znor;
					//vec
					if (p.HP[5] == 0 || p.HP[6] == 0) { //p.zvec.x()
						p.vec = p.zvec.Scale(p.ptr->loc[p.ptr->wheelframe[0]].x() * sin(p.yadd) * ((p.HP[6] == 0) - (p.HP[5] == 0)));
					}
					else {
						p.vec = p.zvec.Scale(p.speed);
					}
					//
					p.wheelrad[0] += p.speed / frate; //
					p.wheelrad[1] = -p.wheelrad[0] * 2 + p.yrad * 5;
					p.wheelrad[2] = -p.wheelrad[0] * 2 - p.yrad * 5;
					//
					p.body->SetLinearVelocity(b2Vec2(p.vec.x(), p.vec.z()));
					p.body->SetAngularVelocity(p.yadd);
				}
				/*物理演算*/
				world->Step(1.0f / frate, 1, 1);
				for (auto& p : player) {
					p.yrad = p.body->GetAngle();
					if (p.speed > 0)
						differential(p.speed, sqrt(pow(p.body->GetLinearVelocity().x, 2) + pow(p.body->GetLinearVelocity().y, 2)), 0.01f);
					else
						differential(p.speed, -sqrt(pow(p.body->GetLinearVelocity().x, 2) + pow(p.body->GetLinearVelocity().y, 2)), 0.01f);

					b2Vec2 tmpb2 = b2Vec2(
					    (M_GR / frate) * VDot(VGet(0, -1.f, 0), (p.obj.frame(7 + 1) - p.obj.frame(7)).Norm()),
					    (M_GR / frate) * (p.nor % VGet(0, 1.f, 0)));

					for (auto& f : p.foot) {
						size_t i = 0;
						VECTOR_ref vects;
						for (auto& w : p.ptr->wheelframe) {
							vects = VECTOR_ref(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w + 1))) +
								VECTOR_ref(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w)));
							if (vects.x() * f.LR > 0)
								f.Wheel[i++].body->SetTransform(b2Vec2(vects.z(), vects.y()), 0.f);
						}
						i = 0;
						for (auto& w : p.ptr->youdoframe) {
							vects = VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w));
							if (vects.x() * f.LR > 0)
								f.Yudo[i++].body->SetTransform(b2Vec2(vects.z(), vects.y()), 0.f);
						}
						for (auto& t : f.Foot)
							t.body->SetLinearVelocity(tmpb2); //

						f.world->Step(1.0f / frate, 3, 3);
						for (auto& t : f.Foot)
							t.pos = VGet(t.pos.x(), t.body->GetPosition().y, t.body->GetPosition().x);
					}
				}
				/*砲撃その他*/
				for (auto& p : player) {
					//地形判定
					{
						const auto HitPoly = mapparts->get_gnd_hit(p.pos + VGet(0.0f, 2.0f, 0.0f), p.pos + VGet(0.0f, -0.05f, 0.0f));
						if (HitPoly.HitFlag) {
							p.yace = 0.0f;
							p.pos = VGet(p.body->GetPosition().x, HitPoly.HitPosition.y, p.body->GetPosition().y);
							mapparts->set_normal(&p.xnor, &p.znor, p.pos);
							p.nor = VTransform(VGet(0, 1.f, 0), MMult(MGetRotX(p.xnor), MGetRotZ(p.znor)));
							/*0.1km/h以内の時かキーを押していないときに減速*/
							if (((0.1f / 3.6f) / frate) < -p.speed && (p.move & KEY_GOBACK_) == 0 ||//バック
							    ((0.1f / 3.6f) / frate) < p.speed && (p.move & KEY_GOFLONT) == 0 ||//前進
							    ((0.1f / 3.6f) / frate) > abs(p.speed) && (p.move & KEY_GOBACK_) == 0 && (p.move & KEY_GOFLONT) == 0)//停止
								p.speed *= 0.95f;
							/*turn*/
							if ((p.move & KEY_GOLEFT_) == 0 && (p.move & KEY_GORIGHT) == 0)
								p.yadd *= 0.9f;
							/*track*/
							mapparts->draw_map_track(p);
						}
						else {
							p.yadd *= 0.95f;
							p.pos = VGet(p.body->GetPosition().x, p.pos.y() + p.yace, p.body->GetPosition().y);
							p.yace += m_ac(frate);
						}
					}
					//サウンド
					ChangeVolumeSoundMem(int(std::min<float>(64.f * abs(p.speed / p.ptr->speed_flont[0]), 64.f) * parts->get_se_vol()), p.se[0].get());
					for (size_t i = 27; i < 29; ++i)
						ChangeVolumeSoundMem(int(std::min<float>(64.f * abs(p.speed / p.ptr->speed_flont[0]), 64.f) * parts->get_se_vol()), p.se[i].get());
					ChangeVolumeSoundMem(int(128.f * p.gun_turn * parts->get_se_vol()), p.se[31].get());
					differential(p.gun_turn, (p.gunrad_rec - p.gunrad).size() / p.ptr->gun_RD, 0.05f);
					p.gunrad_rec = p.gunrad;

					for (auto& s : p.se)
						if (CheckSoundMem(s.get()) == 1)
							Set3DPositionSoundMem(p.pos.get(), s.get());
					//tree判定
					mapparts->set_hitplayer(p.pos);
					/*車体行列*/
					p.ps_r = MMult(
					    MMult(
						MMult(
						    MGetRotAxis(VGet(cos(p.gunrad.x()), 0, -sin(p.gunrad.x())), sin(deg2rad(p.firerad)) * deg2rad(p.ptr->ammosize[0] * 1000 / 75 * 5)),
						    MGetRotAxis(VGet(cos(p.recorad), 0, -sin(p.recorad)), sin(deg2rad(p.recoall)) * deg2rad(5))),
						MGetRotX(atan(p.inertiax))),
					    MMult(
						MGetRotY(-p.yrad),
						MGetRotVec2(VGet(0, 1.f, 0), p.nor.get()))); //MMult(MGetRotX(p.xnor), MGetRotZ(p.znor))
					p.zvec = VTransform(VGet(0, 0, -1.f), p.ps_r);
					p.ps_m = MMult(p.ps_r, p.pos.Mtrans());
					/*砲塔行列*/
					p.ps_t = MMult(MGetRotY(p.gunrad.x()), p.ptr->loc[p.ptr->turretframe].Mtrans());
					//all
					MV1SetMatrix(p.colobj.get(), p.ps_m);
					MV1SetMatrix(p.obj.get(), p.ps_m);
					//common
					MV1SetFrameUserLocalMatrix(p.obj.get(), p.ptr->turretframe, p.ps_t);
					MV1SetFrameUserLocalMatrix(p.colobj.get(), 2, p.ps_t);
					for (int guns = 0; guns < gunc; ++guns) {
						auto mtemp = MMult(MMult(MGetRotX(p.gunrad.y()), (p.ptr->loc[p.ptr->gunframe[guns]] - p.ptr->loc[p.ptr->turretframe]).Mtrans()), p.ps_t);
						MV1SetFrameUserLocalMatrix(p.obj.get(), p.ptr->gunframe[guns], mtemp);
						if (guns == 0)
							MV1SetFrameUserLocalMatrix(p.colobj.get(), 3, mtemp);
						mtemp = (p.ptr->loc[p.ptr->gunframe[guns] + 1] - p.ptr->loc[p.ptr->gunframe[guns]] + VGet(0, 0, p.Gun[guns].fired)).Mtrans();
						MV1SetFrameUserLocalMatrix(p.obj.get(), p.ptr->gunframe[guns] + 1, mtemp);
						if (guns == 0)
							MV1SetFrameUserLocalMatrix(p.colobj.get(), 3 + 1, mtemp);
					}
					for (auto& w : p.ptr->wheelframe) {
						MV1ResetFrameUserLocalMatrix(p.obj.get(), w);
						const auto HitPoly = mapparts->get_gnd_hit(p.obj.frame(w) + p.nor.Scale(1.0f), p.obj.frame(w) + p.nor.Scale(-0.2f));
						if (HitPoly.HitFlag)
							p.Springs[w] = std::min<float>(p.Springs[w] + 1.0f / frate, 1.0f - (VECTOR_ref(HitPoly.HitPosition) - p.obj.frame(w) - p.nor.Scale(1.0f)).size());
						else
							p.Springs[w] = std::max<float>(p.Springs[w] - 0.2f / frate, -0.2f);
						MV1SetFrameUserLocalMatrix(p.obj.get(), w, (p.ptr->loc[w] + p.nor.Scale(p.Springs[w])).Mtrans());
						MV1SetFrameUserLocalMatrix(p.obj.get(), w + 1, MMult(MGetRotX(p.wheelrad[signbit(p.ptr->loc[w + 1].x()) + 1]), (p.ptr->loc[w + 1] - p.ptr->loc[w]).Mtrans()));
					}
					for (auto& w : p.ptr->youdoframe)
						MV1SetFrameUserLocalMatrix(p.obj.get(), w, MMult(MGetRotX(p.wheelrad[signbit(p.ptr->loc[w].x()) + 1]), p.ptr->loc[w].Mtrans()));
					for (auto& w : p.ptr->kidoframe)
						MV1SetFrameUserLocalMatrix(p.obj.get(), w, MMult(MGetRotX(p.wheelrad[signbit(p.ptr->loc[w].x()) + 1]), p.ptr->loc[w].Mtrans()));
					for (auto& f : p.foot) {
						size_t i = 0;
						for (auto& w : p.ptr->upsizeframe) {
							float xw = VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w)).x;
							if (xw * f.LR > 0) {
								f.Foot[i].pos = VGet(xw, f.Foot[i].pos.y(), f.Foot[i].pos.z());
								MV1SetFrameUserLocalMatrix(p.obj.get(), w, f.Foot[i].pos.Mtrans());
								++i;
							}
						}
					}
					/*collition*/
					for (int i = 0; i < p.ptr->colmodel.mesh_num(); ++i)
						MV1RefreshCollInfo(p.colobj.get(), -1, i);
					/*反動*/
					if (p.Gun[0].loadcnt > 0) {
						if (p.firerad < 180)
							if (p.firerad <= 90)
								p.firerad += 900 / (int)frate;
							else
								p.firerad += 180 / (int)frate;
						else
							p.firerad = 180;
					}

					for (size_t guns = 0; guns < gunc; ++guns) {
						if (p.Gun[guns].fired >= 0.01f)
							p.Gun[guns].fired *= 0.95f;
						if (p.Gun[guns].loadcnt == 0) {
							if ((p.move & (KEY_SHOTCAN << guns)) != 0) {
								auto& a = p.Gun[guns].Ammo[p.Gun[guns].useammo];
								a.flug = true;
								a.speed = p.ptr->gun_speed[p.ammotype] / frate;
								a.pene = p.ptr->pene[p.ammotype];
								a.pos = p.obj.frame(p.ptr->gunframe[guns]).get();
								a.repos = a.pos;
								a.cnt = 0;

								const auto v = p.obj.frame(p.ptr->gunframe[guns] + 1) - VECTOR_ref(a.pos);
								const auto y = atan2(v.x(), v.z()) + deg2rad((float)(GetRand(p.ptr->accuracy[guns] * 2) - p.ptr->accuracy[guns]) / 10000.f);
								const auto x = atan2(-v.y(), std::hypot(v.x(), v.z())) - deg2rad((float)(GetRand(p.ptr->accuracy[guns] * 2) - p.ptr->accuracy[guns]) / 10000.f);
								a.vec = VGet(cos(x) * sin(y), -sin(x), cos(x) * cos(y));
								//
								++p.Gun[guns].useammo %= ammoc;
								++p.Gun[guns].loadcnt;
								if (guns == 0) {
									set_effect(&p.effcs[ef_fire], p.obj.frame(p.ptr->gunframe[guns] + 1).get(), (p.obj.frame(p.ptr->gunframe[guns] + 1) - p.obj.frame(p.ptr->gunframe[guns])).get());
									p.Gun[guns].fired = 0.5f;
									p.firerad = 0;
									if (p.id == 0) {
										humanparts->start_humananime(2);
										parts->play_sound(1 + GetRand(6));
									}
									PlaySoundMem(p.se[size_t(2) + GetRand(7)].get(), DX_PLAYTYPE_BACK, TRUE);
								}
								else {
									set_effect(&(p.effcs[ef_gun]), p.obj.frame(p.ptr->gunframe[guns] + 1).get(), VGet(0, 0, 0));
									p.Gun[guns].fired = 0.0f;
									PlaySoundMem(p.se[1].get(), DX_PLAYTYPE_BACK, TRUE);
								}
							}
						}
						else {
							++p.Gun[guns].loadcnt;
							if (p.Gun[guns].loadcnt >= p.ptr->reloadtime[guns]) {
								p.Gun[guns].loadcnt = 0;
								if (p.id == 0 && guns == 0)
									parts->play_sound(8 + GetRand(4));
							} //装てん完了
						}
						for (auto& c : p.Gun[guns].Ammo)
							if (c.flug) {
								c.repos = c.pos;
								c.pos += c.vec.Scale(c.speed);
								const auto HitPoly = mapparts->get_gnd_hit(c.repos, c.pos);
								if (HitPoly.HitFlag)
									c.pos = HitPoly.HitPosition;
								if (!get_reco(p, player, c, guns))
									if (HitPoly.HitFlag) {
										set_effect(&p.effcs[ef_gndhit + guns * (ef_gndhit2 - ef_gndhit)], HitPoly.HitPosition, HitPoly.Normal);
										c.vec += VScale(HitPoly.Normal, (c.vec % HitPoly.Normal) * -2.0f);
										c.pos = c.vec.Scale(0.01f) + HitPoly.HitPosition;
										//c.pene /= 2.0f;
										c.speed /= 2.f;
									}
								c.vec = VGet(c.vec.x(), c.vec.y() + m_ac(frate), c.vec.z());
								c.pene -= 1.0f / frate;
								c.speed -= 5.f / frate;
								c.cnt++;
								if (c.cnt > (frate * 3.f) || c.speed <= 0.f)
									c.flug = false; //3秒で消える
							}
					}
					if (p.recoadd) {
						if (p.recoall < 180) {
							if (p.id == 0 && p.recoall == 0)
								uiparts->set_reco();
							if (p.recoall <= 90)
								p.recoall += 900 / (int)frate;
							else
								p.recoall += 180 / (int)frate;
						}
						else {
							p.recoall = 0;
							p.recoadd = false;
						}
					}
					if (p.hitadd) {
						if (p.id == 0)
							humanparts->start_humanvoice(0);
						player[p.hitid].effcs[ef_smoke1].handle = parts->get_effHandle(ef_smoke1).Play3D();
						p.hitadd = false;
					}
				}
				/*轍更新*/
				mapparts->set_map_track();
				/*human*/
				humanparts->set_humanmove(player[0], parts->get_view_r());
				/*effect*/
				for (auto& p : player) {
					for (int i = 0; i < efs_user; ++i)
						if (i != ef_smoke1 && i != ef_smoke2 && i != ef_smoke3)
							set_pos_effect(&p.effcs[i], parts->get_effHandle(i));
					p.effcs[ef_smoke1].handle.SetPos(p.obj.frame(p.ptr->engineframe));
					p.effcs[ef_smoke2].handle.SetPos(p.obj.frame(p.ptr->smokeframe[0]));
					p.effcs[ef_smoke3].handle.SetPos(p.obj.frame(p.ptr->smokeframe[1]));
				}
				UpdateEffekseer3D();
			}
			/*視点*/
			if (aim.flug) {
				cam = player[0].obj.frame(player[0].ptr->gunframe[0]);
				view = player[0].obj.frame(player[0].ptr->gunframe[0]);
				float getdists;
				getdist(
				    view,
				    (player[0].obj.frame(player[0].ptr->gunframe[0] + 1) - player[0].obj.frame(player[0].ptr->gunframe[0])).Norm(),
				    aim_r,
				    getdists,
				    player[0].ptr->gun_speed[player[0].ammotype],
				    frate);
				upvec = player[0].nor;
			}
			else {
				if (!parts->get_in()) {
					if (keyget[19]) {
						cam = player[0].obj.frame(7);
						view = player[0].obj.frame(7 + 1);
						upvec = player[0].nor;
					}
					else {
						cam = player[0].pos + parts->get_view_pos() + VGet(0, 2, 0);
						view = player[0].pos + VGet(0, 4, 0);
						const auto HitPoly = mapparts->get_gnd_hit(cam.get(), view.get());
						if (HitPoly.HitFlag)
							cam = HitPoly.HitPosition;
						cam += VGet(0, 2, 0);
						upvec = VGet(0, 1, 0);
					}
				}
				else {
					cam = humanparts->get_campos();
					view = humanparts->get_neckpos();
					upvec = player[0].nor;
				}
			}

			mapparts->set_camerapos(cam, view, upvec, rat_r);
			/*shadow*/
			mapparts->set_map_shadow_near(parts->get_view_r().z());
			/*draw*/
			/*map*/
			if (map.flug) {
				SetDrawScreen(minimap);
				ClearDrawScreen();
				DrawExtendGraph(x_r(420), y_r(0), x_r(1500), y_r(1080), mapparts->get_minmap().get(), FALSE);
				//進軍
				for (auto& p : player) {
					if (p.id == 0)
						continue;
					DrawLine(x_(p.pos.x()), y_(p.pos.z()), x_(p.waypos[p.waynow].x()), y_(p.waypos[p.waynow].z()), c_ff0000, 3);
					for (int i = int(p.waynow); i < waypc - 1; ++i)
						DrawLine(x_(p.waypos[i].x()), y_(p.waypos[i].z()), x_(p.waypos[i + 1].x()), y_(p.waypos[i + 1].z()), GetColor(255, 255 * i / waypc, 0), 3);
				}
				for (auto& p : player)
					DrawCircle(x_(p.pos.x()), y_(p.pos.z()), 5, (p.type == TEAM) ? (p.HP[0] == 0) ? c_008000 : c_00ff00 : (p.HP[0] == 0) ? c_800000 : c_ff0000, TRUE);

				for (auto& p : player) {
					//味方
					if (p.id < teamc) {
						//ステータス
						const auto c = (p.id == waysel)
								   ? (p.HP[0] == 0) ? c_c8c800 : c_ffff00
								   : (p.id == choose)
									 ? c_c0ff00
									 : (p.HP[0] == 0) ? c_808080 : c_00ff00;
						DrawBox(x_r(132), y_r(162 + p.id * 24), x_r(324), y_r(180 + p.id * 24), c, TRUE);
						DrawFormatStringToHandle(x_r(132), y_r(162 + p.id * 24), c_ffffff, parts->get_font(0), " %s", p.ptr->name.c_str());

						//進軍パラメータ
						for (size_t i = 0; i < p.wayselect; i++)
							DrawBox(x_r(348 + i * 12), y_r(162 + p.id * 24), x_r(356 + i * 12), y_r(180 + p.id * 24), c_3232ff, TRUE);
					}
					//敵
					else {
						//ステータス
						DrawBox(x_r(1500), y_r(162 + (p.id - teamc) * 24), x_r(1692), y_r(180 + (p.id - teamc) * 24), (p.HP[0] == 0) ? c_808080 : c_ff0000, TRUE);
						DrawFormatStringToHandle(x_r(1500), y_r(162 + (p.id - teamc) * 24), c_ffffff, parts->get_font(0), " %s", p.ptr->name.c_str());
					}
				}
			}
			/*main*/
			else {
				uiparts->end_way(); //debug0//0
				uiparts->end_way(); //debug1//0
				uiparts->end_way(); //debug2//0

				/*sky*/
				if (!parts->get_in() || aim.flug) {
					SetDrawScreen(skyscreen);
					mapparts->draw_map_sky();
				}
				/*near*/
				SetDrawScreen(mainscreen);
				ClearDrawScreen();
				if (aim.flug)
					setcv(0.06f + rat_r / 2, 2000.0f, cam, view, upvec, 45.0f / rat_r);
				else {
					if (keyget[19])
						setcv(0.1f, 2000.0f, cam, view, upvec, 45.0f);
					else
						setcv(0.16f + parts->get_view_r().z(), 2000.0f, cam, view, upvec, 45.0f / rat_r);
				}
				//----------------------------------------------------------
				if (aim.flug) {
					auto& p = player[0];
					auto v = p.obj.frame(p.ptr->gunframe[0]);
					getdist(v, (p.obj.frame(p.ptr->gunframe[0] + 1) - p.obj.frame(p.ptr->gunframe[0])).Norm(), aim_r, aimdist, p.ptr->gun_speed[p.ammotype], frate);
					aimpos = ConvWorldPosToScreenPos(v.get());
				}
				//pos

				for (auto& p : player) {
					p.iconpos = ConvWorldPosToScreenPos((p.pos + VGet(0, (p.pos - player[0].pos).size() / 40 + 6, 0)).get());

					if (CheckCameraViewClip_Box((p.pos + VGet(-5, 0, -5)).get(), (p.pos + VGet(5, 3, 5)).get()) == TRUE)
						pssort[p.id] = pair(p.id, (float)map_x);
					else
						pssort[p.id] = pair(p.id, (p.pos - cam).size());
				}
				std::sort(pssort.begin(), pssort.end(), [](const pair& x, const pair& y) { return x.second > y.second; });

				//effect
				Effekseer_Sync3DSetting();
				//---------------------------------------------------------------
				if (!parts->get_in() || aim.flug) {
					DrawGraph(0, 0, skyscreen, FALSE); //sky
					//main
					ShadowMap_DrawSetup(mapparts->get_map_shadow_seminear());
					for (auto& tt : pssort) {
						if (tt.second == (float)map_x)
							continue;
						if (tt.second < (10.0f * float(parts->get_shadex()) * parts->get_view_r().z() + 20.0f))
							break;
						MV1DrawMesh(player[tt.first].obj.get(), 0);
						for (int i = 1; i < player[tt.first].obj.mesh_num(); ++i)
							if (player[tt.first].HP[i + 4] > 0)
								MV1DrawMesh(player[tt.first].obj.get(), i);
					}
					ShadowMap_DrawEnd();

					ShadowMap_DrawSetup(mapparts->get_map_shadow_near());
					if (player[0].HP[0] > 0)
						humanparts->draw_human(0);

					for (auto& tt : pssort) {
						if (tt.second > (10.0f * float(parts->get_shadex()) * parts->get_view_r().z() + 20.0f))
							continue;
						MV1DrawMesh(player[tt.first].obj.get(), 0);
						for (int i = 1; i < player[tt.first].obj.mesh_num(); ++i) {
							if (i < 3)
								MV1SetFrameTextureAddressTransform(player[tt.first].obj.get(), 0, 0.0, player[tt.first].wheelrad[i], 1.0, 1.0, 0.5, 0.5, 0.0);
							if (i == 3)
								MV1ResetFrameTextureAddressTransform(player[tt.first].obj.get(), 0);
							if (player[tt.first].HP[i + 4] > 0)
								MV1DrawMesh(player[tt.first].obj.get(), i);
						}
					}
					ShadowMap_DrawEnd();

					if (player[0].HP[0] > 0)
						humanparts->draw_human(0);

					mapparts->ready_shadow();
					mapparts->draw_map_model();
					for (auto& tt : pssort) {
						if (tt.second == (float)map_x)
							continue;
						if (tt.first != 0 || (tt.first == 0 && !aim.flug)) {
							auto& p = player[tt.first];
							MV1DrawMesh(p.obj.get(), 0);
							for (int i = 1; i < p.obj.mesh_num(); ++i) {
								if (i < 3)
									MV1SetFrameTextureAddressTransform(p.obj.get(), 0, 0.0, p.wheelrad[i], 1.0, 1.0, 0.5, 0.5, 0.0);
								if (i == 3)
									MV1ResetFrameTextureAddressTransform(p.obj.get(), 0);
								if (p.HP[i + 4] > 0)
									MV1DrawMesh(p.obj.get(), i);
							}
							//MV1DrawModel(p.colobj.get());/*コリジョンモデルを出すとき*/
							for (int i = 0; i < p.hit.size(); ++i)
								if (p.hit[i].flug) {
									MV1SetRotationZYAxis(p.hit[i].pic.get(), (p.colobj.frame(11 + 3 * i) - p.colobj.frame(9 + 3 * i)).get(), (p.colobj.frame(10 + 3 * i) - p.colobj.frame(9 + 3 * i)).get(), 0.f);
									MV1SetPosition(p.hit[i].pic.get(), (p.colobj.frame(9 + 3 * i) + (p.colobj.frame(10 + 3 * i) - p.colobj.frame(9 + 3 * i)).Scale(0.005f)).get());
									MV1DrawFrame(p.hit[i].pic.get(), p.hit[i].use);
								}
						}
					}
					//grass
					mapparts->draw_grass();
					//effect
					DrawEffekseer3D();
					//ammo
					uiparts->end_way(); //debug3//0
					SetUseLighting(FALSE);
					SetFogEnable(FALSE);
					for (auto& p : player)
						for (size_t guns = 0; guns < gunc; ++guns)
							for (size_t i = 0; i < ammoc; ++i)
								if (p.Gun[guns].Ammo[i].flug) {
									SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f * std::min<float>(1.f, 4.f * p.Gun[guns].Ammo[i].speed / (p.ptr->gun_speed[p.ammotype] / frate))));
									DrawCapsule3D(p.Gun[guns].Ammo[i].pos.get(), p.Gun[guns].Ammo[i].repos.get(), p.ptr->ammosize[guns] * ((p.Gun[guns].Ammo[i].pos - cam).size() / 60.f), 4, p.Gun[guns].Ammo[i].color, c_ffffff, TRUE);
								}
					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
					SetFogEnable(TRUE);
					SetUseLighting(TRUE);

					//tree
					mapparts->draw_trees();
					DrawEffekseer3D();
					mapparts->exit_shadow();

					uiparts->end_way(); //debug4//0
				}
				else
					humanparts->draw_humanall();
			}
			SetDrawScreen(DX_SCREEN_BACK);
			ClearDrawScreen();
			if (map.flug)
				DrawGraph(0, 0, minimap, FALSE); /*指揮*/
			else {
				/*通常*/
				DrawGraph(0, 0, mainscreen, FALSE);
				/*ブルーム*/
				if (!parts->get_in() && parts->get_usehost()) {
					GraphFilterBlt(mainscreen, HighBrightScreen, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_LESS, 210, TRUE, c_000000, 255);
					GraphFilterBlt(HighBrightScreen, GaussScreen, DX_GRAPH_FILTER_DOWN_SCALE, EXTEND);
					GraphFilter(GaussScreen, DX_GRAPH_FILTER_GAUSS, 16, 1000);
					SetDrawMode(DX_DRAWMODE_BILINEAR);
					SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
					DrawExtendGraph(0, 0, dispx, dispy, GaussScreen, FALSE);
					DrawExtendGraph(0, 0, dispx, dispy, GaussScreen, FALSE);
					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
				}
				/*UI*/
				if (aim.flug)
					uiparts->draw_sight(aimpos, rat_r, aimdist, parts->get_font(0)); /*照準器*/
				else
					/*アイコン*/
					for (auto& tt : pssort) {
						if (tt.first == 0 || tt.second == (float)map_x)
							continue;
						uiparts->draw_icon(player[tt.first], parts->get_font(0));
					}
				if (keyget[19] && !aim.flug)
					uiparts->draw_drive(); /*ドライバー視点*/

				uiparts->draw_ui(selfammo, parts->get_view_r().y(), parts->get_font(0)); /*main*/
			}
			/*debug*/
			//DrawFormatStringToHandle(x_r(18), y_r(1062), c_ffffff, parts->get_font(0), "start-stop(%.2fms)", (float)stop_w / 1000.f);
			uiparts->debug(fps, (float)(GetNowHiPerformanceCount() - waits) / 1000.0f);
			//
			parts->Screen_Flip(waits);
		}
		//delete
		mapparts->delete_map();
		humanparts->delete_human();
		for (auto& p : player) {
			/*エフェクト*/
			for (auto&& e : p.effcs)
				e.handle.Dispose();
			/*Box2D*/
			delete p.playerfix->GetUserData();
			p.playerfix->SetUserData(NULL);
			/**/
			p.obj.Dispose();
			p.colobj.Dispose();
			for (auto& h : p.hit)
				h.pic.Dispose();
			for (auto& s : p.se)
				s.Dispose();
			for (size_t guns = 0; guns < gunc; ++guns)
				p.Gun[guns].Ammo.clear();
			p.Springs.clear();
			p.HP.clear();
			p.hitssort.clear();

			for (auto& t : p.foot) {
				for (auto& f : t.Foot) {
					delete f.playerfix->GetUserData();
					f.playerfix->SetUserData(NULL);
				}
				for (auto& f : t.Wheel) {
					delete f.playerfix->GetUserData();
					f.playerfix->SetUserData(NULL);
				}
				for (auto& f : t.Yudo) {
					delete f.playerfix->GetUserData();
					f.playerfix->SetUserData(NULL);
				}
			}
		}
		pssort.clear();
		player.clear();
	} while (!out);
	/*終了*/
	return 0;
}
