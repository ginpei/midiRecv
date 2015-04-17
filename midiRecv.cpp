/***************************************************************
	
	midiRecv.cpp
	
		MIDI Data Receiver
			Ver 1.1 for Windows XP
		(C) �����M���y�C 2006/02/17
	
	MIDI IN �f�o�C�X���m�F���A��������ΑI���������̂��J���܂��B
	���̌�f�[�^����M���A��͂��ĕ\�����܂��B
	
	��M�����̗���́A
	  1. midiInOpen()		�C�ӂ̃f�o�C�X���J��
	  2.   midiInStart()	�Ώۃf�o�C�X�����M�J�n
	  3.     midiInProc()	����Ă����f�[�^����M
	  4.   midiInStop()		�Ώۃf�o�C�X����̎�M��~
	  5. midiInClose()		�f�o�C�X�����
	�Ƃ����������ł��BmidiInProc() �̓v���V�[�W���ŁA���O�ŏ���
	�ď�������K�v������܂��BWindows �v���O���~���O���������
	�̂�����Ȃ炷���킩��Ǝv���܂��B
	
	�Ȃ��u���b�Z�[�W�v�ƌ������ꍇ�ɂ� OS ���s�̃��b�Z�[�W��
	MIDI �̃��b�Z�[�W�A����ɂ��̃v���O�������\�����镶�͂Ƃ���
	��܂��̂ŁA�������画�f����悤�ɂ��ĉ������B
	
	���Ȃ݂Ƀ\�[�X�R�[�h�� TAB ���� 4 �ł��B
	
***************************************************************/

#include <stdio.h>		// STanDard Input/Output
#include <windows.h>	// ���ꂪ�Ȃ��� mmsystem.h �������Ȃ�
#include <mmsystem.h>	// Include file for Multimedia API's
#include <conio.h>		// kbhit(), getch() �p

// getch() �Ŏg���L�[�R�[�h
#define VK_ENTER	13	// Enter
#define VK_ESC		27	// Esc

// �^�C���X�^���v���������̃��b�Z�[�W
#define USAGE_ON_RUNNING	"\nEnter �L�[�Ń^�C���X�^���v�̏������AEsc �L�[�ŏI�����܂��B\nTIME STAMP  St Dt Dt : [  ch] Messages\n     0.000  -- -- -- : [--ch] ----\n"


// MIDI ��M�֘A�̊֐�
void CALLBACK MidiInProc(HMIDIIN, UINT, DWORD, DWORD, DWORD);	// MIDI IN �R�[���o�b�N�֐�
void OnMimData(DWORD, DWORD);	// MIDI �f�[�^��M



/*--------------------------------------------------------------
	�G���g�� �|�C���g
--------------------------------------------------------------*/
int main()
{
	UINT		wNumDevs;	// �f�o�C�X��
	UINT		wSelDev;	// �f�o�C�X���ʎq
	HMIDIIN		hMidiIn;	// �f�o�C�X�n���h��
	MIDIINCAPSA MidiInCaps;	// �f�o�C�X���\����
	MMRESULT	res;		// �߂�l�i�[
	
///////	��ʂ��N���A
	system("cls");
	
///////	�f�o�C�X���m�F
	printf("Number of MIDI IN devices is ...");
	printf("\b\b\b   \b\b\b: %d\n", (wNumDevs=midiInGetNumDevs()));
	if(wNumDevs < 1) {
		printf("MIDI IN �f�o�C�X������܂���B\n");
		printf("�I�����܂��B�����L�[�������ĉ������B\n");
		getch();
		exit(1);
	}
	
///////	�f�o�C�X����\�����đI��
	char inp[4];
	for(int i=0; i<(int)wNumDevs; i++) {
		if(midiInGetDevCaps(i, &MidiInCaps, sizeof(MidiInCaps)) != MMSYSERR_NOERROR) {
			printf("���炩�̃G���[���������܂����B\n");	// �����͎蔲���̃G���[����
			printf("�I�����܂��B�����L�[�������ĉ������B\n");
			getch();
			exit(1);
		}
		printf("[%d]\n", i+1);
		printf("      manufacturer ID : %d\n", MidiInCaps.wMid);
		printf("           product ID : %d\n", MidiInCaps.wPid);
		printf("version of the driver : %d\n", MidiInCaps.vDriverVersion);
		printf("         product name : %s\n", MidiInCaps.szPname);
		// �����o�͂����ЂƂAdwSupport (functionality supported by driver) ������ꍇ������
	}
	if(wNumDevs > 1 ) {
		printf("�g�p����f�o�C�X�̔ԍ�����͂��ĉ�����:");
		fflush(stdin);
		fgets(inp, sizeof(inp), stdin);
		wSelDev = atoi(inp)-1;
	} else {
		wSelDev = 0;	// �ЂƂ����f�o�C�X���Ȃ���΁A�����I�ɂ����I��
	}
	
///////	MIDI IN �I�[�v��
	switch((res = midiInOpen(&hMidiIn, wSelDev, (DWORD)MidiInProc, NULL, CALLBACK_FUNCTION))) {
	case MMSYSERR_NOERROR:
		break;
	case MMSYSERR_ALLOCATED:
		printf("�w�肳�ꂽ���\�[�X�͊��Ɋ��蓖�Ă��Ă��܂�\n");
		break;
	case MMSYSERR_BADDEVICEID:
		printf("�w�肳�ꂽ�f�o�C�X���ʎq %d �͔͈͊O�ł��B\n", wSelDev);
		break;
	case MMSYSERR_INVALFLAG:
		printf("dwFlags �p�����[�^�Ŏw�肳�ꂽ�t���O�͖����ł��B\n");
		break;
	case MMSYSERR_INVALPARAM:
		printf("�w�肳�ꂽ�|�C���^�܂��͍\���͖̂����ł��B\n");
		break;
	case MMSYSERR_NOMEM:
		printf("�V�X�e���̓����������蓖�Ă��Ȃ����A�܂��̓��b�N�ł��܂���B\n");
		break;
	default:
		printf("�s���ȃG���[�ł��B\n");
		break;
	}
	if(res != MMSYSERR_NOERROR) {
		printf("�I�����܂��B\n");
		printf("�I�����܂��B�����L�[�������ĉ������B\n");
		getch();
		exit(1);
	}
	
///////	�f�o�C�X����̎�t���J�n
	if((res=midiInStart(hMidiIn)) != MMSYSERR_NOERROR) {
		if(res == MMSYSERR_INVALHANDLE)
			printf("�w�肳�ꂽ�f�o�C�X�n���h���͖����ł��B\n");
		else
			printf("�s���ȃG���[�ł��B\n");
		midiInClose(hMidiIn);
		printf("�I�����܂��B�����L�[�������ĉ������B\n");
		getch();
		exit(1);
	}
	printf("MIDI �̓��͂��J�n���܂��B\n");
	printf(USAGE_ON_RUNNING);
	
///////	�������[�v
	while(true) {
	///////	�L�[���삪����܂Ń��[�v
		while(!kbhit()) {
			// �󃋁[�v�ł������ȁH
			// �v���V�[�W���́AOS �����b�Z�[�W�𔭍s�����ꍇ�Ɏ����I�ɌĂяo������s����܂��B
		}
		
	///////	���̓L�[�𔻒f
		unsigned char key = getch();
		if(key == VK_ENTER) {
			// Enter �L�[�Ȃ���͂����������~���čĊJ
			midiInStop(hMidiIn);
			printf(USAGE_ON_RUNNING);
			midiInStart(hMidiIn);
			continue;
		} else if(key == VK_ESC) {
			// Esc �L�[�Ȃ�E�o
			break;
		} else if(key == 0 || key == 0xe0) {
			// ���� Fn �L�[�̏ꍇ�� 2byte �Ȃ̂ł������ getch() ����
			getch();
		}
	}
	
///////	�I������
	midiInStop(hMidiIn);
	midiInClose(hMidiIn);
	
	printf("�I�����܂��B�����L�[�������ĉ������B\n");
	getch();
	
	return 0;
}



/*--------------------------------------------------------------
	MIDI IN �R�[���o�b�N�֐�
	
	midiInOpen() �Ŏw�肵���v���V�[�W���BOS ��MIDI IN�֘A�̃�
	�b�Z�[�W�𔭍s�����ۂɎ����I�ɌĂяo����܂��B
	�v���V�[�W���ɏ����𒼐ڏ����Ƃ����Ⴒ���Ⴗ��̂ŁA���ʂ�
	���b�Z�[�W���Ƃɏ������֐��ɕ����ď����܂��B����� MIM_DATA
	����M�����ꍇ�ɂ̂ݏ�����؂蕪����悤�ɂ��܂����B
--------------------------------------------------------------*/
void CALLBACK MidiInProc(
  HMIDIIN hMidiIn,
  UINT wMsg,
  DWORD dwInstance,
  DWORD dwParam1,
  DWORD dwParam2
) {
	switch(wMsg) {
	case MIM_OPEN:		// ���̓f�o�C�X �I�[�v�� �R�[���o�b�N
		printf("*** MIM_OPEN ***\n");
		break;
	case MIM_CLOSE:		// ���̓f�o�C�X���N���[�Y
		printf("*** MIM_CLOSE ***\n");
		break;
	case MIM_DATA:		// ���̓f�o�C�X �f�[�^ �R�[���o�b�N
		OnMimData(dwParam1, dwParam2);
		break;
	case MIM_LONGDATA:	// ���̓o�b�t�@�̃R�[���o�b�N
		printf("*** MIM_LONGDATA ***\n");
		break;
	case MIM_ERROR:		// ���̓f�o�C�X �G���[ �R�[���o�b�N
		printf("*** MIM_ERROR ***\n");
		break;
	case MIM_LONGERROR:	// �����ȃV�X�e�� �G�N�X�N���[�V�u ���b�Z�[�W�ɑ΂���R�[���o�b�N
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
	MIDI �f�[�^�̃V���[�g ���b�Z�[�W��M
	
	�V���[�g ���b�Z�[�W����M�����ۂɌĂ΂��֐��ł��B��M��
	�b�Z�[�W dwData ����͂��ĕ\�����܂��B
	dwData �̉��ʃo�C�g���珇�ɃX�e�[�^�X �o�C�g�A�f�[�^ �o�C
	�g 1 �A�f�[�^ �o�C�g 2 �A��o�C�g�Ƃ��܂��B
	�X�e�[�^�X �o�C�g�̏�� 4bits �� 1111 �ł���΃V�X�e�����b
	�Z�[�W�A1000�`1110 �ł���΃`���l�� ���b�Z�[�W�ł��B�܂���
	�� 4bits �̓V�X�e�� ���b�Z�[�W�Ȃ炻�̎�ނ��A�`���l�����b
	�Z�[�W�̏ꍇ�͔��s�`���l����\���Ă��܂��B
	�ڂ����͋K�i�̉�����Ȃǂ��Q�Ƃ��ĉ������B
--------------------------------------------------------------*/
#define SKIP_TIMING_CLOCK	1	// ���b�Z�[�W 0xF8=timing clock �͖�������
void OnMimData(DWORD dwData, DWORD dwTimeStamp)
{
	unsigned char st, dt1, dt2;	// ��M�f�[�^�i�[�o�b�t�@
	
///////	��M�f�[�^�𕪉�
	st = dwData     & 0xFF;	// �X�e�[�^�X �o�C�g
	dt1= dwData>> 8 & 0xFF;	// �f�[�^ �o�C�g�P
	dt2= dwData>>16 & 0xFF;	// �f�[�^ �o�C�g�Q
	
#if SKIP_TIMING_CLOCK
	if(st == 0xF8)
		return;
#endif
	
///////	��M�f�[�^��\��
	printf("%10.3f  %02X %02X %02X : ", (double)dwTimeStamp/1000, st, dt1, dt2);
	
///////	��M�f�[�^�𕪐�
	if(st>>4 == 0xF) {
	///////	�V�X�e�� ���b�Z�[�W
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
	/////// �`���l�� ���b�Z�[�W
		printf("[%2dch] ", (st &0xF) +1);
		static const char keys[] = "CDEFGAB";	// ���K
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

