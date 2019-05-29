#ifndef PROGS_H
#define PROGS_H

#include <map>
#include <set>
#include <utility>
#include <memory>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include <limits>
#include <cmath>

struct insdata
{
    uint8_t         data[11];
    int8_t          finetune;
    bool            diff;
    bool operator==(const insdata &b) const
    {
        return (std::memcmp(data, b.data, 11) == 0) && (finetune == b.finetune) && (diff == b.diff);
    }
    bool operator!=(const insdata &b) const
    {
        return !operator==(b);
    }
    bool operator<(const insdata &b) const
    {
        int c = std::memcmp(data, b.data, 11);
        if(c != 0) return c < 0;
        if(finetune != b.finetune) return finetune < b.finetune;
        if(diff != b.diff) return (diff) == (!b.diff);
        return 0;
    }
    bool operator>(const insdata &b) const
    {
        return !operator<(b) && operator!=(b);
    }
};

inline bool equal_approx(double const a, double const b)
{
    double const epsilon(std::numeric_limits<double>::epsilon() * 100);
    double const scale(1.0);
    return std::fabs(a - b) < epsilon * (scale + (std::max)(std::fabs(a), std::fabs(b)));
}

struct ins
{
    enum { Flag_Pseudo4op = 0x01, Flag_NoSound = 0x02, Flag_Real4op = 0x04 };

    enum { Flag_RM_BassDrum  = 0x08, Flag_RM_Snare = 0x10, Flag_RM_TomTom = 0x18,
           Flag_RM_Cymbal = 0x20, Flag_RM_HiHat = 0x28, Mask_RhythmMode = 0x38 };

    size_t insno1, insno2;
    unsigned char notenum;
    bool pseudo4op;
    bool real4op;
    uint32_t rhythmModeDrum;
    double voice2_fine_tune;
    int8_t midi_velocity_offset;

    bool operator==(const ins &b) const
    {
        return notenum == b.notenum
               && insno1 == b.insno1
               && insno2 == b.insno2
               && pseudo4op == b.pseudo4op
               && real4op == b.real4op
               && rhythmModeDrum == b.rhythmModeDrum
               && equal_approx(voice2_fine_tune, b.voice2_fine_tune)
               && midi_velocity_offset == b.midi_velocity_offset;
    }
    bool operator< (const ins &b) const
    {
        if(insno1 != b.insno1) return insno1 < b.insno1;
        if(insno2 != b.insno2) return insno2 < b.insno2;
        if(notenum != b.notenum) return notenum < b.notenum;
        if(pseudo4op != b.pseudo4op) return pseudo4op < b.pseudo4op;
        if(real4op != b.real4op) return real4op < b.real4op;
        if(rhythmModeDrum != b.rhythmModeDrum) return rhythmModeDrum < b.rhythmModeDrum;
        if(!equal_approx(voice2_fine_tune, b.voice2_fine_tune)) return voice2_fine_tune < b.voice2_fine_tune;
        if(midi_velocity_offset != b.midi_velocity_offset) return midi_velocity_offset < b.midi_velocity_offset;
        return 0;
    }
    bool operator!=(const ins &b) const
    {
        return !operator==(b);
    }
};

enum VolumesModels
{
    VOLUME_Generic,
    VOLUME_CMF,
    VOLUME_DMX,
    VOLUME_APOGEE,
    VOLUME_9X
};

struct AdlBankSetup
{
    int     volumeModel;
    bool    deepTremolo;
    bool    deepVibrato;
    bool    scaleModulators;
};

typedef std::map<insdata, std::pair<size_t, std::set<std::string> > > InstrumentDataTab;
extern InstrumentDataTab insdatatab;

typedef std::map<ins, std::pair<size_t, std::set<std::string> > > InstrumentsData;
extern InstrumentsData instab;

typedef std::map<size_t, std::map<size_t, size_t> > InstProgsData;
extern InstProgsData progs;

typedef std::map<size_t, AdlBankSetup> BankSetupData;
extern BankSetupData banksetup;

extern std::vector<std::string> banknames;

//static std::map<unsigned, std::map<unsigned, unsigned> > Correlate;
//extern unsigned maxvalues[30];


struct BanksDump
{
    struct BankEntry
    {
        uint_fast32_t bankId;

        /* Global OPL flags */
        typedef enum WOPLFileFlags
        {
            /* Enable Deep-Tremolo flag */
            WOPL_FLAG_DEEP_TREMOLO = 0x01,
            /* Enable Deep-Vibrato flag */
            WOPL_FLAG_DEEP_VIBRATO = 0x02
        } WOPLFileFlags;

        /* Volume scaling model implemented in the libADLMIDI */
        typedef enum WOPL_VolumeModel
        {
            WOPL_VM_Generic = 0,
            WOPL_VM_Native,
            WOPL_VM_DMX,
            WOPL_VM_Apogee,
            WOPL_VM_Win9x
        } WOPL_VolumeModel;

        /**
         * @brief Suggested bank setup in dependence from a driver that does use of this
         */
        enum BankSetup
        {
            SETUP_Generic = 0x0300,
            SETUP_Win9X   = 0x0304,
            SETUP_DMX     = 0x0002,
            SETUP_Apogee  = 0x0003,
            SETUP_AIL     = 0x0300,
            SETUP_IBK     = 0x0301,
            SETUP_IMF     = 0x0200,
            SETUP_CMF     = 0x0201
        };

        uint_fast16_t       bankSetup; // 0xAABB, AA - OPL flags, BB - Volume model
        std::vector<size_t> melodic;
        std::vector<size_t> percussion;
    };

    struct MidiBank
    {
        uint_fast32_t midiBankId;
        uint_fast8_t msb, lsb;
        int_fast32_t instruments[128];
    };

    struct InstrumentEntry
    {
        uint_fast32_t instId;

        typedef enum WOPL_InstrumentFlags
        {
            /* Is two-operator single-voice instrument (no flags) */
            WOPL_Ins_2op        = 0x00,
            /* Is true four-operator instrument */
            WOPL_Ins_4op        = 0x01,
            /* Is pseudo four-operator (two 2-operator voices) instrument */
            WOPL_Ins_Pseudo4op  = 0x02,
            /* Is a blank instrument entry */
            WOPL_Ins_IsBlank    = 0x04,

            /* RythmMode flags mask */
            WOPL_RhythmModeMask  = 0x38,

            /* Mask of the flags range */
            WOPL_Ins_ALL_MASK   = 0x07
        } WOPL_InstrumentFlags;

        typedef enum WOPL_RhythmMode
        {
            /* RythmMode: BassDrum */
            WOPL_RM_BassDrum  = 0x08,
            /* RythmMode: Snare */
            WOPL_RM_Snare     = 0x10,
            /* RythmMode: TomTom */
            WOPL_RM_TomTom    = 0x18,
            /* RythmMode: Cymbell */
            WOPL_RM_Cymbal   = 0x20,
            /* RythmMode: HiHat */
            WOPL_RM_HiHat     = 0x28
        } WOPL_RhythmMode;

        uint_fast8_t  noteOffset1;
        uint_fast8_t  noteOffset2;
        int_fast8_t   midiVelocityOffset;
        uint_fast8_t  percussionKeyNumber;
        uint_fast32_t instFlags;
        double        secondVoiceDetune;
        /*
            2op: modulator1, carrier1, feedback1
            2vo: modulator1, carrier1, modulator2, carrier2, feedback(1+2)
            4op: modulator1, carrier1, modulator2, carrier2, feedback1
        */
        //! Contains FeedBack-Connection for both operators 0xBBAA - AA - first, BB - second
        int16_t   fbConn;
        size_t    ops[5] = {-1, -1, -1, -1, -1};
        int64_t   delay_on_ms;
        int64_t   delay_off_ms;
    };

    struct Operator
    {
        uint_fast32_t d_E862;
        uint_fast32_t d_40;
    };

    std::vector<BankEntry>       banks;
    std::vector<MidiBank>        midiBanks;
    std::vector<InstrumentEntry> instruments;
    std::vector<Operator>        operators;
};



void SetBank(size_t bank, unsigned patch, size_t insno);
void SetBankSetup(size_t bank, const AdlBankSetup &setup);

/* 2op voice instrument */
size_t InsertIns(const insdata &id, ins &in,
                 const std::string &name, const std::string &name2);

/* 4op voice instrument or double-voice 2-op instrument */
size_t InsertIns(const insdata &id, const insdata &id2, ins &in,
                 const std::string &name, const std::string &name2,
                 bool oneVoice = false);

size_t InsertNoSoundIns();
insdata MakeNoSoundIns();

#endif // PROGS_H
