/******************************************************************************
/ BR_MidiUtil.h
/
/ Copyright (c) 2014 Dominik Martin Drzic
/ http://forum.cockos.com/member.php?u=27094
/ https://code.google.com/p/sws-extension
/
/ Permission is hereby granted, free of charge, to any person obtaining a copy
/ of this software and associated documentation files (the "Software"), to deal
/ in the Software without restriction, including without limitation the rights to
/ use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
/ of the Software, and to permit persons to whom the Software is furnished to
/ do so, subject to the following conditions:
/
/ The above copyright notice and this permission notice shall be included in all
/ copies or substantial portions of the Software.
/
/ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/ EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
/ OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/ NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/ HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/ WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/ FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
/ OTHER DEALINGS IN THE SOFTWARE.
/
******************************************************************************/
#pragma once

/******************************************************************************
* MIDI timebase - this is how Reaper stores timebase settings internally      *
******************************************************************************/
enum BR_MidiTimebase
{
	PROJECT_BEATS = 0,
	PROJECT_SYNC  = 1,
	PROJECT_TIME  = 2,
	SOURCE_BEATS  = 4
};

/******************************************************************************
* MIDI noteshow - this is how Reaper stores noteshow settings internally      *
******************************************************************************/
enum BR_MidiNoteshow
{
	SHOW_ALL_NOTES             = 0,
	HIDE_UNUSED_NOTES          = 1,
	HIDE_UNUSED_UNNAMED_NOTES  = 2
};

/******************************************************************************
* MIDI CC lanes - this is how Reaper stores CC lanes internally               *
******************************************************************************/
enum BR_MidiVelLanes
{
	CC_VELOCITY         = -1,
	CC_PITCH            = 128,
	CC_PROGRAM          = 129,
	CC_CHANNEL_PRESSURE = 130,
	CC_BANK_SELECT      = 131,
	CC_TEXT_EVENTS      = 132,
	CC_SYSEX            = 133,
	CC_14BIT_START      = 134
};

/******************************************************************************
* MIDI status bytes - purely for readability                                  *
******************************************************************************/
enum BR_MidiStatusBytes
{
	STATUS_NOTE_ON          = 0x90,
	STATUS_POLY_PRESSURE    = 0xA0,
	STATUS_CC               = 0xB0,
	STATUS_PROGRAM          = 0xC0,
	STATUS_CHANNEL_PRESSURE = 0xD0,
	STATUS_PITCH            = 0xE0,
	STATUS_SYS              = 0xF0
};

/******************************************************************************
* Various constants related to position and dimension of MIDI editor elements *
******************************************************************************/
const int MIDI_RULER_H                 = 44;
const int MIDI_LANE_DIVIDER_H          = 9;
const int MIDI_LANE_TOP_GAP            = 4;
const int MIDI_BLACK_KEYS_W            = 73;

const int INLINE_MIDI_MIN_H            = 32;
const int INLINE_MIDI_MIN_NOTEVIEW_H   = 24;
const int INLINE_MIDI_LANE_DIVIDER_H   = 6;
const int INLINE_MIDI_KEYBOARD_W       = 12;
const int INLINE_MIDI_TOP_BAR_H        = 17;

/******************************************************************************
* Class for managing normal or inline MIDI editor (read-only for now)         *
******************************************************************************/
class BR_MidiEditor
{
public:
	BR_MidiEditor ();                     // last active main MIDI editor
	BR_MidiEditor (void* midiEditor);     // main MIDI editor
	BR_MidiEditor (MediaItem_Take* take); // inline MIDI editor

	/* Various MIDI editor view options */
	MediaItem_Take* GetActiveTake ();
	double GetStartPos ();            // can be ppq or time - depends on timebase
	double GetHZoom ();               // can be ppq or time - depends on timebase
	int GetPPQ ();
	int GetVPos ();                   // not really working for inline midi editor (can be 0 which means it's auto adjusted by take height, or is completely wrong if notes are hidden)
	int GetVZoom ();                  // not really working for inline midi editor (can be 0 which means it's auto adjusted by take height, or is completely wrong if notes are hidden)
	int GetNoteshow ();               // see BR_MidiNoteshow
	int GetTimebase ();               // see BR_MidiTimeBase
	int GetPianoRoll ();
	int GetDrawChannel ();

	/* CC lanes */
	int CountCCLanes ();
	int GetCCLane (int idx);
	int GetCCLaneHeight (int idx);
	int GetLastClickedCCLane ();
	int FindCCLane (int lane);
	bool IsCCLaneVisible (int lane);

	/* Event filter */
	bool IsNoteVisible (MediaItem_Take* take, int id);
	bool IsCCVisible (MediaItem_Take* take, int id);
	bool IsSysVisible (MediaItem_Take* take, int id);
	bool IsChannelVisible (int channel);

	/* Misc */
	void* GetEditor ();

	/* Check if MIDI editor data is valid - should call right after creating the object  */
	bool IsValid ();

private:
	bool Build ();
	bool CheckVisibility (MediaItem_Take* take, int chanMsg, double position, double end, int channel, int param, int value);

	MediaItem_Take* m_take;
	void* m_midiEditor;
	double m_startPos, m_hZoom;
	int m_vPos, m_vZoom, m_noteshow, m_timebase, m_pianoroll, m_drawChannel, m_ccLanesCount, m_ppq, m_lastLane;
	int m_filterChannel, m_filterEventType, m_filterEventParamLo, m_filterEventParamHi, m_filterEventValLo, m_filterEventValHi;
	double m_filterEventPosRepeat, m_filterEventPosLo, m_filterEventPosHi, m_filterEventLenLo, m_filterEventLenHi;
	bool m_filterEnabled, m_filterInverted, m_filterEventParam, m_filterEventVal, m_filterEventPos, m_filterEventLen;
	bool m_valid;
	vector<int> m_ccLanes, m_ccLanesHeight;
};

/******************************************************************************
* Class for saving and restoring TIME positioning info of an item and         *
* all of it's MIDI events                                                     *
******************************************************************************/
class BR_MidiItemTimePos
{
public:
	BR_MidiItemTimePos (MediaItem* item, bool deleteSavedEvents = true);
	void Restore (bool clearCurrentEvents = true, double offset = 0);

private:
	struct MidiTake
	{
		struct NoteEvent
		{
			NoteEvent (MediaItem_Take* take, int id);
			void InsertEvent (MediaItem_Take* take, double offset);
			bool selected, muted;
			double pos, end;
			int chan, pitch, vel;
		};
		struct CCEvent
		{
			CCEvent (MediaItem_Take* take, int id);
			void InsertEvent (MediaItem_Take* take, double offset);
			bool selected, muted;
			double pos;
			int chanMsg, chan, msg2, msg3;
		};
		struct SysEvent
		{
			SysEvent (MediaItem_Take* take, int id);
			void InsertEvent (MediaItem_Take* take, double offset);
			bool selected, muted;
			double pos;
			int type, msg_sz;
			WDL_TypedBuf<char> msg;
		};

		MidiTake (MediaItem_Take* take, int noteCount = 0, int ccCount = 0, int sysCount = 0);
		vector<NoteEvent> noteEvents;
		vector<CCEvent> ccEvents;
		vector<SysEvent> sysEvents;
		MediaItem_Take* take;
	};
	MediaItem* item;
	double position, length, timeBase;
	vector<BR_MidiItemTimePos::MidiTake> savedMidiTakes;
};

/******************************************************************************
* Mouse cursor                                                                *
******************************************************************************/
double ME_PositionAtMouseCursor (bool checkRuler, bool checkCCLanes);

/******************************************************************************
* Miscellaneous                                                               *
******************************************************************************/
vector<int> GetUsedNamedNotes (void* midiEditor, MediaItem_Take* take, bool used, bool named, int channelForNames);
vector<int> GetSelectedNotes (MediaItem_Take* take);
vector<int> MuteSelectedNotes (MediaItem_Take* take); // returns previous mute state of all notes
set<int> GetUsedCCLanes (void* midiEditor, int detect14bit); // detect14bit: 0-> don't detect 14-bit, 1->detect partial 14-bit (count both 14 bit lanes and their counterparts) 2->detect full 14-bit (detect only if all CCs that make it have exactly same time positions)
double EffectiveMidiTakeLength (MediaItem_Take* take, bool ignoreMutedEvents, bool ignoreTextEvents);
double EffectiveMidiTakeStart (MediaItem_Take* take, bool ignoreMutedEvents, bool ignoreTextEvents);
double GetStartOfMeasure (MediaItem_Take* take, double ppqPos); // working versions of MIDI_GetPPQPos_StartOfMeasure
double GetEndOfMeasure (MediaItem_Take* take, double ppqPos);   // and MIDI_GetPPQPos_EndOfMeasure
void SetMutedNotes (MediaItem_Take* take, const vector<int>& muteStatus);
void SetSelectedNotes (MediaItem_Take* take, const vector<int>& selectedNotes, bool unselectOthers);
void UnselectAllEvents (MediaItem_Take* take, int lane);
bool AreAllNotesUnselected (MediaItem_Take* take);
bool IsMidi (MediaItem_Take* take, bool* inProject = NULL);
bool IsOpenInInlineEditor (MediaItem_Take* take);
bool IsMidiNoteBlack (int note);
bool IsVelLaneValid (int lane);
int FindFirstSelectedNote (MediaItem_Take* take, BR_MidiEditor* midiEditorFilterSettings); // Pass midiEditorFilterSettings in case you
int FindFirstSelectedCC   (MediaItem_Take* take, BR_MidiEditor* midiEditorFilterSettings); // want to check events through MIDI filter
int GetMIDIFilePPQ (const char* fp);
int GetLastClickedVelLane (void* midiEditor);
int MapVelLaneToReaScriptCC (int lane); // CC format follows ReaScript scheme: 0-127=CC, 0x100|(0-31)=14-bit CC, 0x200=velocity, 0x201=pitch,
int MapReaScriptCCToVelLane (int cc);   // 0x202=program, 0x203=channel pressure, 0x204=bank/program select, 0x205=text, 0x206=sysex