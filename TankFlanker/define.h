﻿#pragma once

#ifndef INCLUDED_define_h_
#define INCLUDED_define_h_
#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include <windows.h>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include "Box2D/Box2D.h"
#include "useful.h"
#include <array>
#include <algorithm>
#include <memory>
#include <vector>
#include <cstring>
#include <string_view>
#include <cstdint>
#include <optional>
#include <array>
#include <iostream>
#include <fstream>

#include "DXLib_vec.hpp"
#include "MV1ModelHandle.hpp"
#include "EffekseerEffectHandle.hpp"
#include "SoundHandle.hpp"
#include "GraphHandle.hpp"
#include "FontHandle.hpp"


using std::size_t;
using std::uint8_t;
inline const int dispx = (GetSystemMetrics(SM_CXSCREEN)); /*描画X*/
inline const int dispy = (GetSystemMetrics(SM_CYSCREEN)); /*描画Y*/
constexpr float M_GR = -9.8f;				  /*重力加速度*/
constexpr size_t waypc = 4;				  /*移動確保数*/
constexpr size_t ammoc = 64;				  /*砲弾確保数*/
#define map_x 1000					  /*マップサイズX*/
#define map_y 1000					  /*マップサイズY*/
#define TEAM 1						  /*味方ID*/
#define ENEMY 2						  /*敵ID*/
#define EXTEND 4					  /*ブルーム用*/
constexpr size_t gunc = 2;				  /*銃、砲の数*/
#define divi 2						  /*人の物理処理*/

/*構造体*/
enum animeid {
	ANIME_L1 = 0,
	ANIME_L2 = 1,
	ANIME_L3 = 2,
	ANIME_LtoR = 3,
	ANIME_R = 4,
	ANIME_RtoL = 5,
	ANIME_nom = 6,
	ANIME_sit = 7,
	ANIME_eye = 8,
	ANIME_voi = 9,
	ANIME_out = 10,
	ANIME_voice = 2 //ボイス数
};
enum cpu {
	CPU_NOMAL = 0,
	CPU_CHARGE = 1
};
enum Key {
	KEY_GOFLONT = 0x001,
	KEY_GOBACK_ = 0x002,
	KEY_GOLEFT_ = 0x004,
	KEY_GORIGHT = 0x008,
	KEY_TURNLFT = 0x010,
	KEY_TURNRIT = 0x020,
	KEY_TURNUP_ = 0x040,
	KEY_TURNDWN = 0x080,
	KEY_SHOTCAN = 0x100,
	KEY_SHOTGAN = 0x200
};
enum Bone {
	//転輪、履帯用フレーム
	bone_wheel = 11,
	//砲塔内
	bone_hatch = 5,	   //カメラ
	bone_in_turret = 6 //人配置フレーム
};
enum Effect {
	ef_fire = 0,
	ef_reco = 1,
	ef_bomb = 2,
	ef_smoke1 = 3,
	ef_smoke2 = 4,
	ef_gndhit = 5,
	ef_gun = 6,
	ef_gndhit2 = 7,
	ef_reco2 = 8,
	effects = 9,   //読み込む
	ef_smoke3 = 9, //読み込まない
	efs_user = 10
};

struct ammos {
	bool flug{ false };
	int cnt = 0;
	int color = 0;
	float speed = 0.f, pene = 0.f;
	VECTOR_ref pos, repos, vec;
};
struct EffectS {
	bool flug{ false };		 /**/
	Effekseer3DPlayingHandle handle; /**/
	VECTOR_ref pos;			 /**/
	VECTOR_ref nor;			 /**/
};
struct Hit {
	bool flug{ false }; /**/
	int use{ 0 };	    /*使用フレーム*/
	MV1ModelHandle pic; /*弾痕モデル*/
};
struct switches {
	bool flug{ false };
	uint8_t cnt{ 0 };
};
struct vehicle {
	std::string name;		  /*名前*/
	int countryc;			  /*国*/
	MV1ModelHandle model;		  /*モデル*/
	MV1ModelHandle colmodel;	  /*コリジョン*/
	MV1ModelHandle inmodel;		  /*内装*/
	std::array<float, 4> speed_flont; /*前進*/
	std::array<float, 4> speed_back;  /*後退*/
	float vehicle_RD = 0.0f;	  /*旋回速度*/
	float armer[4] = { 0 };		  /*装甲*/
	bool gun_lim_LR = 0;		  /*砲塔限定旋回の有無*/
	float gun_lim_[4] = { 0.f };	  /*砲塔旋回制限*/
	float gun_RD = 0.0f;		  /*砲塔旋回速度*/
	float gun_speed[3] = { 0.0f };	  /*弾速*/
	float pene[3] = { 0.0f };	  /*貫通*/
	int ammotype[3] = { 0 };	  /*弾種*/
	std::vector<VECTOR_ref> loc;	  /*フレームの元座標*/
	VECTOR_ref min;			  /*box2D用フレーム*/
	VECTOR_ref max;			  /*box2D用フレーム*/
	int turretframe;		  /*砲塔フレーム*/
	std::array<int, gunc> gunframe;	  /*銃フレーム*/
	std::array<int, gunc> reloadtime; /*リロードタイム*/
	std::array<float, gunc> ammosize; /*砲口径*/
	std::array<int, gunc> accuracy;	  /*砲精度*/
	std::vector<int> youdoframe;
	std::vector<int> wheelframe;
	std::array<int, 2> kidoframe;
	std::array<int, 2> smokeframe;
	std::vector<int> upsizeframe;
	int engineframe;
};
static_assert(std::is_move_constructible_v<vehicle>);
namespace std {
	template <>
	struct default_delete<b2Body> {
		void operator()(b2Body* body) const {
			body->GetWorld()->DestroyBody(body);
		}
	};
};
struct players {
	/*情報*/
	int id{ 0 };
	int use{ 0 }; /*使用車両*/
	vehicle* ptr; /*vehicle*/

	MV1ModelHandle obj;    /*モデル*/
	MV1ModelHandle colobj; /*コリジョン*/

	char type{ 0 };		     /*敵味方識別*/
	std::vector<SoundHandle> se; /*SE*/

	int move{ 0 };	/*キー操作*/
	VECTOR_ref pos; /*座標*/
	MATRIX ps_r;    /*車体旋回行列*/
	MATRIX ps_m;    /*車体全体行列*/
	MATRIX ps_t;	/*砲塔行列*/
	//std::vector<MATRIX> ps_all;					/*行列*/
	float yace{ 0.f };				/*加速度*/
	float speed{ 0.f }, speedrec{ 0.f };		/*速度関連*/
	float accel{ 0.f };				/*加速度*/
	VECTOR_ref vec;					/*移動ベクトル*/
	float xnor{ 0.f }, znor{ 0.f }, znorrec{ 0.f }; /*法線角度*/
	VECTOR_ref nor;					/*法線*/
	VECTOR_ref zvec;				/*前向きベクトル*/
	float yrad{ 0.f };				/*角度*/
	float yadd{ 0.f };				/*角速度*/
	int recoall{ 0 };				/*弾き角度*/
	int firerad{ 0 };				/*反動角度*/
	float recorad{ 0.f };				/*弾き反動*/
	/*cpu関連*/
	std::optional<size_t> atkf;	      /*cpuのヘイト*/
	int aim{ 0 };			      /*ヘイトの変更カウント*/
	size_t wayselect{ 0 }, waynow{ 0 };   /**/
	std::array<VECTOR_ref, waypc> waypos; /*ウェイポイント*/
	std::array<int, waypc> wayspd;	      /*速度指定*/
	int state{ 0 };			      /*ステータス*/
	/**/
	struct Guns {
		std::vector<ammos> Ammo; /*確保する弾(arrayでもいい？)*/
		int loadcnt{ 0 };	 /*装てんカウンター*/
		size_t useammo{};	 /*使用弾*/
		float fired{ 0.f };	 /*駐退*/
	} Gun[gunc];
	/**/
	int gear{ 0 };		  /*変速*/
	unsigned int gearu{ 0 };  /*キー*/
	unsigned int geard{ 0 };  /*キー*/
	float inertiax, inertiaz; /*慣性*/
	float wheelrad[3]{ 0.f }; /*履帯の送り、転輪旋回*/
	VECTOR_ref gunrad;	  /*砲角度*/
	VECTOR_ref gunrad_rec;	  /*砲角度*/
	float gun_turn{ 0.f };
	/*弾関連*/
	int ammotype{ 0 };     /*弾種*/
	bool recoadd{ false }; /*弾きフラグ*/
	bool hitadd{ false };  /*命中フラグ*/
	size_t hitid{ 0 };
	VECTOR_ref iconpos;		     /*UI用*/
	std::array<EffectS, efs_user> effcs; /*effect*/
	std::vector<float> Springs;	     /*スプリング*/
	std::vector<short> HP;		     /*ライフ*/
	std::vector<pair> hitssort;	     /*当たった順番*/
	/*弾痕*/
	int hitbuf;		/*使用弾痕*/
	std::array<Hit, 3> hit; /**/
	//確保
	std::vector<MV1_COLL_RESULT_POLY> hitres;
	/*box2d*/
	std::unique_ptr<b2Body> body; /**/
	b2Fixture* playerfix;	      /**/
	/*足物理*/
	struct Foots {
		std::unique_ptr<b2Body> body; /**/
		b2Fixture* playerfix;	      /**/
		VECTOR_ref pos;		      /**/
	};
	struct FootWorld {
		b2World* world;		       /**/
		b2RevoluteJointDef f_jointDef; /**/
		std::vector<Foots> Foot;       /**/
		std::vector<Foots> Wheel;      /**/
		std::vector<Foots> Yudo;       /**/
		float LR;
	};
	std::array<FootWorld, 2> foot; /**/
};
/*CLASS*/
class Myclass {
private:
	/*setting*/
	bool usegrab{ false };	  /*人の物理演算のオフ、オン*/
	unsigned char ANTI{ 1 };  /*アンチエイリアス倍率*/
	bool YSync{ true };	  /*垂直同期*/
	float f_rate{ 60.f };	  /*fps*/
	bool windowmode{ false }; /*ウィンドウor全画面*/
	float drawdist{ 100.0f }; /*木の描画距離*/
	int gndx = 8;		  /*地面のクオリティ*/
	int shadex = 3;		  /*影のクオリティ*/
	bool USEHOST{ false };	  /**/
	float se_vol{ 1.f };	  /**/
	/**/
	std::vector<vehicle> vecs;		 /*車輛情報*/
	VECTOR_ref view, view_r;		 /*通常視点の角度、距離*/
	std::vector<int> fonts;			 /*フォント*/
	std::array<SoundHandle, 13> se_;	 /*効果音*/
	std::array<GraphHandle, 4> ui_reload;	 /*UI用*/
	EffekseerEffectHandle effHndle[effects]; /*エフェクトリソース*/
public:
	Myclass();
	bool get_usegrab(void) { return usegrab; }
	int get_gndx(void) { return gndx; }
	int get_shadex(void) { return shadex; }
	float get_drawdist(void) { return drawdist; }
	float get_f_rate(void) { return f_rate; }
	bool get_usehost(void) { return USEHOST; }
	float get_se_vol(void) { return se_vol; }

	void autoset_option(void);

	void write_option(void);

	template <typename... Args>
	void set_fonts(Args&&... args) {
		SetUseASyncLoadFlag(true);
		(this->fonts.emplace(this->fonts.end(), DxLib::CreateFontToHandle(NULL, x_r(args), y_r(args / 3), DX_FONTTYPE_ANTIALIASING_EDGE)), ...);
		SetUseASyncLoadFlag(false);
	} //(必要なフォント数,サイズ1,サイズ2, ...)

	bool set_veh(void);
	int window_choosev(void); //車両選択
	void set_viewrad(VECTOR_ref vv);
	void set_view_r(int wheel, bool life);
	void Screen_Flip(LONGLONG waits);
	~Myclass();
	void set_se_vol(unsigned char size);
	void play_sound(int p1);
	auto& get_ui2() & { return ui_reload; }
	const auto& get_ui2() const& { return ui_reload; }
	int get_font(int p1) { return fonts[p1]; } //フォントハンドル取り出し
	VECTOR_ref get_view_r(void) { return view_r; }
	const auto get_in(void) { return view_r.z() == 0.1f; }
	VECTOR_ref get_view_pos(void) { return VScale(VGet(sin(view_r.y()) * cos(view_r.x()), sin(view_r.x()), cos(view_r.y()) * cos(view_r.x())), 15.0f * view_r.z()); }
	EffekseerEffectHandle& get_effHandle(int p1) noexcept { return effHndle[p1]; }
	const EffekseerEffectHandle& get_effHandle(int p1) const noexcept { return effHndle[p1]; }
	vehicle* get_vehicle(int p1) { return &vecs[p1]; }
};
class HUMANS {
private:
	struct Hmod {
		MV1ModelHandle obj;
		int amine[ANIME_out]{ 0 };
		float alltime[ANIME_out]{ 0.f };
		std::array<SoundHandle, ANIME_voice> vsound;
	};

	struct humans {
		char vflug{ 0 };

		MV1ModelHandle obj;
		std::array<int, ANIME_out> amine{};
		std::array<float, ANIME_out> time{};
		std::array<float, ANIME_out> alltime{};
		std::array<float, ANIME_out> per{};

		int neck{ 0 };
		VECTOR_ref nvec;
		float voicetime{ 0.f };

		float voicealltime[ANIME_voice]{ 0 };
		int voices[ANIME_voice]{ 0 };
		std::array<SoundHandle, ANIME_voice> vsound;
	};
	bool usegrab{ false };		 /*人の物理演算のオフ、オン*/
	float f_rate{ 60.f };		 /*fps*/
	MV1ModelHandle inmodel_handle;	 //中モデル
	bool in_f{ false };		 //中描画スイッチ
	size_t inflames;		 //inmodelのフレーム数
	std::vector<humans> hum;	 /**/
	std::vector<VECTOR_ref> locin;	 /*inmodelのフレーム*/
	std::vector<VECTOR_ref> pos_old; /*inmodelの前回のフレーム*/
	std::vector<std::string> name;	 /**/
	bool first;			 //初回フラグ
	std::vector<Hmod> model;

public:
	HUMANS(bool useg, float frates);
	bool set_humans(const MV1ModelHandle& inmod);
	void set_humanvc_vol(unsigned char size);
	void set_humanmove(const players& player, VECTOR_ref rad);
	void draw_human(size_t p1);
	void draw_humanall();
	void delete_human(void);
	void start_humanvoice(std::int8_t p1);
	void start_humananime(int p1);
	VECTOR_ref get_neckpos() { return hum[0].obj.frame(hum[0].neck); }
	VECTOR_ref get_campos() { return inmodel_handle.frame(bone_hatch); }
};
class MAPS {
private:
	/*setting*/
	int groundx;
	float drawdist;
	int shadowx;
	/**/
	size_t treec = 750; /*木の数*/
	struct trees {
		MV1ModelHandle mnear;		   /**/
		MV1ModelHandle mfar;		   /**/
		std::vector<MV1ModelHandle> nears; /**/
		std::vector<MV1ModelHandle> fars;  /**/

		std::vector<pair> treesort;
		std::vector<VECTOR_ref> pos;
		std::vector<VECTOR_ref> rad;
		std::vector<bool> hit;
	} tree;
	int shadow_seminear;			  /*shadow中距離*/
	int shadow_near;			  /*shadow近距離*/
	int shadow_far;				  /*shadowマップ用*/
	MV1ModelHandle m_model, minmap;		  /*mapモデル*/
	GraphHandle texp, texo, texn, texm, texl; /*mapテクスチャ*/
	MV1ModelHandle sky_model;		  /*skyモデル*/
	GraphHandle sky_sun;			  /*sunpic*/
	VECTOR_ref lightvec;			  /*light方向*/
	/*grass*/
	int grasss = 10000;		/*grassの数*/
	std::vector<VERTEX3D> grassver; /**/
	std::vector<DWORD> grassind;	/**/
	int VerBuf, IndexBuf;		/**/
	MV1ModelHandle grass;		/*grassモデル*/
	GraphHandle graph;		/*画像ハンドル*/
	int IndexNum, VerNum;		/**/
	GraphHandle GgHandle;		/**/
	int vnum, pnum;			/**/
	MV1_REF_POLYGONLIST RefMesh;	/**/
	//cam
	VECTOR_ref camera, viewv, upv; /**/
	float rat;		       /**/
public:
	MAPS(int map_size, float draw_dist, int shadow_size);
	void set_map_readyb(size_t set);
	bool set_map_ready(void);
	void set_camerapos(VECTOR_ref pos, VECTOR_ref vec, VECTOR_ref up, float ratio);
	void set_map_shadow_near(float vier_r);
	void draw_map_track(const players& player);
	void draw_map_model(void);
	void set_map_track(void);
	void draw_map_sky(void);
	void delete_map(void);

	void ready_shadow(void);
	void exit_shadow(void);
	void set_normal(float* xnor, float* znor, VECTOR_ref position); //地面に沿わせる
	auto& get_minmap() & { return texp; }
	const auto& get_minmap() const& noexcept { return texp; }

	int get_map_shadow_near() { return shadow_near; }
	int get_map_shadow_seminear() { return shadow_seminear; }

	MV1_COLL_RESULT_POLY get_gnd_hit(VECTOR_ref startpos, VECTOR_ref endpos) { return MV1CollCheck_Line(m_model.get(), 0, startpos.get(), endpos.get()); }
	void set_hitplayer(VECTOR_ref pos);
	void draw_trees(void);
	void draw_grass(void);
};
class UIS {
private:
	struct country {
		std::array<GraphHandle, 8> ui_sight;
	};
	/**/
	std::array<GraphHandle, 4> ui_reload; /*弾UI*/
	GraphHandle ui_compass;		      /*UI*/
	std::vector<GraphHandle> UI_body;     /*UI*/
	std::vector<GraphHandle> UI_turret;   /*UI*/
	std::vector<country> UI_main;	      /*国別UI*/
	size_t countries = 1;		      /*国数*/
	float gearf = 0.f;		      /*変速*/
	float recs = 0.f;		      /*跳弾表現用*/
	players* pplayer;		      /*playerdata*/

	/*debug*/
	float deb[60][6 + 1]{ 0.f };
	LONGLONG waypoint{ 0 }; /*時間取得*/
	float waydeb[6]{ 0 };
	size_t seldeb{};

public:
	UIS();
	void draw_load(void);	       /*ロード画面*/
	void set_state(players* play); /*使用するポインタの指定*/
	void set_reco(void);	       /*反射スイッチ*/
	void draw_drive();
	void draw_icon(players& p, int font);
	void draw_sight(VECTOR_ref aimpos, float ratio, float dist, int font); /*照準UI*/
	void draw_ui(uint8_t selfammo[], float y_v, int font);			    /*メインUI*/
	/*debug*/
	void put_way(void);
	void end_way(void);
	void debug(float fps, float time);
};
/**/
void setcv(float neard, float fard, VECTOR_ref cam, VECTOR_ref view, VECTOR_ref up, float fov);		     //カメラ情報指定
void getdist(VECTOR_ref& startpos, VECTOR_ref vec, float& dist, float& getdists, float speed, float fps); //startposに測距情報を出力
//effect
void set_effect(EffectS* efh, VECTOR_ref pos, VECTOR_ref nor);
void set_pos_effect(EffectS* efh, const EffekseerEffectHandle& handle);
//play_class予定
bool get_reco(players& play, std::vector<players>& tgts, ammos& c, size_t gun_s);
void set_gunrad(players& play, float rat_r);
bool set_shift(players& play);
//
#endif
