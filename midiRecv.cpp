/***************************************************************
	
	midiRecv.cpp
	
		MIDI Data Receiver
			Ver 1.1 for Windows XP
		(C) 高梨ギンペイ 2006/02/17
	
	MIDI IN デバイスを確認し、複数あれば選択したものを開きます。
	その後データを受信し、解析して表示します。
	
	受信処理の流れは、
	  1. midiInOpen()		任意のデバイスを開く
	  2.   midiInStart()	対象デバイスから受信開始
	  3.     midiInProc()	流れてきたデータを受信
	  4.   midiInStop()		対象デバイスからの受信停止
	  5. midiInClose()		デバイスを閉じる
	といった感じです。midiInProc() はプロシージャで、自前で書い
	て処理する必要があります。Windows プログラミングをやった事
	のある方ならすぐわかると思います。
	
	なお「メッセージ」と言った場合には OS 発行のメッセージと
	MIDI のメッセージ、さらにこのプログラムが表示する文章とがあ
	りますので、文脈から判断するようにして下さい。
	
	ちなみにソースコードの TAB 幅は 4 です。
	
***************************************************************/

#include <stdio.h>		// STanDard Input/Output
#include <windows.h>	// これがないと mmsystem.h が動かない
#include <mmsystem.h>	// Include file for Multimedia API's
#include <conio.h>		// kbhit(), getch() 用

// getch() で使うキーコード
#define VK_ENTER	13	// Enter
#define VK_ESC		27	// Esc

// タイムスタンプ初期化時のメッセージ
#define USAGE_ON_RUNNING	"\nEnter キーでタイムスタンプの初期化、Esc キーで終了します。\nTIME STAMP  St Dt Dt : [  ch] Messages\n     0.000  -- -- -- : [--ch] ----\n"


// MIDI 受信関連の関数
void CALLBACK MidiInProc(HMIDIIN, UINT, DWORD, DWORD, DWORD);	// MIDI IN コールバック関数
void OnMimData(DWORD, DWORD);	// MIDI データ受信



/*--------------------------------------------------------------
	エントリ ポイント
--------------------------------------------------------------*/
int main()
{
	UINT		wNumDevs;	// デバイス数
	UINT		wSelDev;	// デバイス識別子
	HMIDIIN		hMidiIn;	// デバイスハンドル
	MIDIINCAPSA MidiInCaps;	// デバイス情報構造体
	MMRESULT	res;		// 戻り値格納
	
///////	画面をクリア
	system("cls");
	
///////	デバイスを確認
	printf("Number of MIDI IN devices is ...");
	printf("\b\b\b   \b\b\b: %d\n", (wNumDevs=midiInGetNumDevs()));
	if(wNumDevs < 1) {
		printf("MIDI IN デバイスがありません。\n");
		printf("終了します。何かキーを押して下さい。\n");
		getch();
		exit(1);
	}
	
///////	デバイス情報を表示して選択
	char inp[4];
	for(int i=0; i<(int)wNumDevs; i++) {
		if(midiInGetDevCaps(i, &MidiInCaps, sizeof(MidiInCaps)) != MMSYSERR_NOERROR) {
			printf("何らかのエラーが発生しました。\n");	// ここは手抜きのエラー処理
			printf("終了します。何かキーを押して下さい。\n");
			getch();
			exit(1);
		}
		printf("[%d]\n", i+1);
		printf("      manufacturer ID : %d\n", MidiInCaps.wMid);
		printf("           product ID : %d\n", MidiInCaps.wPid);
		printf("version of the driver : %d\n", MidiInCaps.vDriverVersion);
		printf("         product name : %s\n", MidiInCaps.szPname);
		// メンバはもうひとつ、dwSupport (functionality supported by driver) がある場合もある
	}
	if(wNumDevs > 1 ) {
		printf("使用するデバイスの番号を入力して下さい:");
		fflush(stdin);
		fgets(inp, sizeof(inp), stdin);
		wSelDev = atoi(inp)-1;
	} else {
		wSelDev = 0;	// ひとつしかデバイスがなければ、自動的にそれを選択
	}
	
///////	MIDI IN オープン
	switch((res = midiInOpen(&hMidiIn, wSelDev, (DWORD)MidiInProc, NULL, CALLBACK_FUNCTION))) {
	case MMSYSERR_NOERROR:
		break;
	case MMSYSERR_ALLOCATED:
		printf("指定されたリソースは既に割り当てられています\n");
		break;
	case MMSYSERR_BADDEVICEID:
		printf("指定されたデバイス識別子 %d は範囲外です。\n", wSelDev);
		break;
	case MMSYSERR_INVALFLAG:
		printf("dwFlags パラメータで指定されたフラグは無効です。\n");
		break;
	case MMSYSERR_INVALPARAM:
		printf("指定されたポインタまたは構造体は無効です。\n");
		break;
	case MMSYSERR_NOMEM:
		printf("システムはメモリを割り当てられないか、またはロックできません。\n");
		break;
	default:
		printf("不明なエラーです。\n");
		break;
	}
	if(res != MMSYSERR_NOERROR) {
		printf("終了します。\n");
		printf("終了します。何かキーを押して下さい。\n");
		getch();
		exit(1);
	}
	
///////	デバイスからの受付を開始
	if((res=midiInStart(hMidiIn)) != MMSYSERR_NOERROR) {
		if(res == MMSYSERR_INVALHANDLE)
			printf("指定されたデバイスハンドルは無効です。\n");
		else
			printf("不明なエラーです。\n");
		midiInClose(hMidiIn);
		printf("終了します。何かキーを押して下さい。\n");
		getch();
		exit(1);
	}
	printf("MIDI の入力を開始します。\n");
	printf(USAGE_ON_RUNNING);
	
///////	無限ループ
	while(true) {
	///////	キー操作があるまでループ
		while(!kbhit()) {
			// 空ループでいいかな？
			// プロシージャは、OS がメッセージを発行した場合に自動的に呼び出され実行されます。
		}
		
	///////	入力キーを判断
		unsigned char key = getch();
		if(key == VK_ENTER) {
			// Enter キーなら入力をいったん停止して再開
			midiInStop(hMidiIn);
			printf(USAGE_ON_RUNNING);
			midiInStart(hMidiIn);
			continue;
		} else if(key == VK_ESC) {
			// Esc キーなら脱出
			break;
		} else if(key == 0 || key == 0xe0) {
			// 矢印と Fn キーの場合は 2byte なのでもう一回 getch() する
			getch();
		}
	}
	
///////	終了処理
	midiInStop(hMidiIn);
	midiInClose(hMidiIn);
	
	printf("終了します。何かキーを押して下さい。\n");
	getch();
	
	return 0;
}



/*--------------------------------------------------------------
	MIDI IN コールバック関数
	
	midiInOpen() で指定したプロシージャ。OS がMIDI IN関連のメ
	ッセージを発行した際に自動的に呼び出されます。
	プロシージャに処理を直接書くとごちゃごちゃするので、普通は
	メッセージごとに処理を関数に分けて書きます。今回は MIM_DATA
	を受信した場合にのみ処理を切り分けるようにしました。
--------------------------------------------------------------*/
void CALLBACK MidiInProc(
  HMIDIIN hMidiIn,
  UINT wMsg,
  DWORD dwInstance,
  DWORD dwParam1,
  DWORD dwParam2
) {
	switch(wMsg) {
	case MIM_OPEN:		// 入力デバイス オープン コールバック
		printf("*** MIM_OPEN ***\n");
		break;
	case MIM_CLOSE:		// 入力デバイスをクローズ
		printf("*** MIM_CLOSE ***\n");
		break;
	case MIM_DATA:		// 入力デバイス データ コールバック
		OnMimData(dwParam1, dwParam2);
		break;
	case MIM_LONGDATA:	// 入力バッファのコールバック
		printf("*** MIM_LONGDATA ***\n");
		break;
	case MIM_ERROR:		// 入力デバイス エラー コールバック
		printf("*** MIM_ERROR ***\n");
		break;
	case MIM_LONGERROR:	// 無効なシステム エクスクルーシブ メッセージに対するコールバック
		printf("*** MIM_LONGERROR ***\n");
		break;
	case MIM_MOREDATA:	// ???
		printf("*** MIM_MOREDATA ***\n");
		break;
	default:
		printf("*** (unknown message) ***\n");
		break;
	}
	return;
}



/*--------------------------------------------------------------
	MIDI データのショート メッセージ受信
	
	ショート メッセージを受信した際に呼ばれる関数です。受信メ
	ッセージ dwData を解析して表示します。
	dwData の下位バイトから順にステータス バイト、データ バイ
	ト 1 、データ バイト 2 、空バイトとします。
	ステータス バイトの上位 4bits が 1111 であればシステムメッ
	セージ、1000～1110 であればチャネル メッセージです。また下
	位 4bits はシステム メッセージならその種類を、チャネルメッ
	セージの場合は発行チャネルを表しています。
	詳しくは規格の解説書などを参照して下さい。
--------------------------------------------------------------*/
#define SKIP_TIMING_CLOCK	1	// メッセージ 0xF8=timing clock は無視する
void OnMimData(DWORD dwData, DWORD dwTimeStamp)
{
	unsigned char st, dt1, dt2;	// 受信データ格納バッファ
	
///////	受信データを分解
	st = dwData     & 0xFF;	// ステータス バイト
	dt1= dwData>> 8 & 0xFF;	// データ バイト１
	dt2= dwData>>16 & 0xFF;	// データ バイト２
	
#if SKIP_TIMING_CLOCK
	if(st == 0xF8)
		return;
#endif
	
///////	受信データを表示
	printf("%10.3f  %02X %02X %02X : ", (double)dwTimeStamp/1000, st, dt1, dt2);
	
///////	受信データを分析
	if(st>>4 == 0xF) {
	///////	システム メッセージ
		switch(st & 0xF) {
		case 0x1:
			printf("MTC quarter frame (Type:%d Val:2d)", dt1>>4&0xF, dt1&0xF);
			break;
		case 0x2:
			printf("song position pointer (Beat:%5d)", dt1+dt2<<7);
			break;
		case 0x3:
			printf("song select (Num:%3d)", dt1);
			break;
		case 0x6:
			printf("tune request");
			break;
		case 0x7:
			printf("end of exclusive");
			break;
		case 0x8:
			printf("timing clock");
			break;
		case 0xA:
			printf("start");
			break;
		case 0xB:
			printf("continue");
			break;
		case 0xC:
			printf("stop");
			break;
		case 0xE:
			printf("active sensing");
			break;
		case 0xF:
			printf("system reset");
			break;
		default:
			printf("(unknown system mes)");
			break;
		}
	} else {
	/////// チャネル メッセージ
		printf("[%2dch] ", (st &0xF) +1);
		static const char keys[] = "CDEFGAB";	// 音階
		switch(st>>4) {
		case 0x8:
			printf("note off* (Note:%c%-2d Vel:%3d)", keys[dt1%7], dt1/7-3, dt2);
			break;
		case 0x9:
			printf("note %s  (Note:%c%-2d Vel:%3d)", (dt2?"on ":"off"), keys[dt1%7], dt1/7-3, dt2);
			break;
		case 0xA:
			printf("polyphonic key pressure (Note:%c%-2d Prss:%3d)", keys[dt1%7], dt1/7-3, dt2);
			break;
		case 0xB:
			printf("control change (Num:%3d Val:%3d)", dt1, dt2);
			break;
		case 0xC:
			printf("program change (Num:%3d)", dt1);
			break;
		case 0xD:
			printf("channel pressure (Prss:%3d)", dt1);
			break;
		case 0xE:
			printf("pitch bend change (Val:%5d)", dt1+dt2<<7);
			break;
		default:
			printf("(unknown channel message)");
			break;
		}
	}
	printf("\n");
	return;
}

