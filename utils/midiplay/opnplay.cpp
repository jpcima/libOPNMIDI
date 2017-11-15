
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <signal.h>

#ifdef DEBUG_DUMP_RAW_STREAM
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#endif

#include <opnmidi.h>

class MutexType
{
    SDL_mutex *mut;
public:
    MutexType() : mut(SDL_CreateMutex()) { }
    ~MutexType()
    {
        SDL_DestroyMutex(mut);
    }
    void Lock()
    {
        SDL_mutexP(mut);
    }
    void Unlock()
    {
        SDL_mutexV(mut);
    }
};


typedef std::deque<int16_t> AudioBuff;
static AudioBuff g_audioBuffer;
static MutexType AudioBuffer_lock;

static void SDL_AudioCallbackX(void *, Uint8 *stream, int len)
{
    SDL_LockAudio();
    short *target = reinterpret_cast<int16_t*>(stream);
    AudioBuffer_lock.Lock();
    size_t ate = size_t(len) / 2; // number of shorts
    if(ate > g_audioBuffer.size()) ate = g_audioBuffer.size();
    for(size_t a = 0; a < ate; ++a)
        target[a] = g_audioBuffer[a];
    g_audioBuffer.erase(g_audioBuffer.begin(), g_audioBuffer.begin() + AudioBuff::difference_type(ate));
    AudioBuffer_lock.Unlock();
    SDL_UnlockAudio();
}

static bool g_playing = true;

void handle_signal(int signal)
{
    if(signal == SIGINT)
    {
        g_playing = false;
        printf("\n\n");
        fflush(stdout);
    }
}


int main(int argc, char **argv)
{
    if(argc < 3 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")
    {
        std::printf(
            "Usage: opnmidi <bankfile>.wopn <midifilename>\n"
            //" -p Enables adlib percussion instrument mode\n"
            //" -t Enables tremolo amplification mode\n"
            //" -v Enables vibrato amplification mode\n"
            " -s Enables scaling of modulator volumes\n"
            " -nl Quit without looping\n"
            " -w Write WAV file rather than playing\n"
        );
        /*
                for(unsigned a=0; a<sizeof(banknames)/sizeof(*banknames); ++a)
                    std::printf("%10s%2u = %s\n",
                        a?"":"Banks:",
                        a,
                        banknames[a]);
        */
        std::printf(
            "     Use banks 2-5 to play Descent \"q\" soundtracks.\n"
            "     Look up the relevant bank number from descent.sng.\n"
            "\n"
            "     The fourth parameter can be used to specify the number\n"
            "     of four-op channels to use. Each four-op channel eats\n"
            "     the room of two regular channels. Use as many as required.\n"
            "     The Doom & Hexen sets require one or two, while\n"
            "     Miles four-op set requires the maximum of numcards*6.\n"
            "\n"
        );
        return 0;
    }

    //const unsigned MaxSamplesAtTime = 512; // 512=dbopl limitation
    // How long is SDL buffer, in seconds?
    // The smaller the value, the more often SDL_AudioCallBack()
    // is called.
    const double AudioBufferLength = 0.08;
    // How much do WE buffer, in seconds? The smaller the value,
    // the more prone to sound chopping we are.
    const double OurHeadRoomLength = 0.1;
    // The lag between visual content and audio content equals
    // the sum of these two buffers.
    SDL_AudioSpec spec;
    SDL_AudioSpec obtained;

    spec.freq     = 44100;
    spec.format   = AUDIO_S16SYS;
    spec.channels = 2;
    spec.samples  = Uint16(spec.freq * AudioBufferLength);
    spec.callback = SDL_AudioCallbackX;

    // Set up SDL
    if(SDL_OpenAudio(&spec, &obtained) < 0)
    {
        std::fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        return 1;
    }

    if(spec.samples != obtained.samples)
        std::fprintf(stderr,
                     "Wanted (samples=%u,rate=%u,channels=%u);\n"
                     "Obtained (samples=%u,rate=%u,channels=%u)\n",
                     spec.samples,    spec.freq,    spec.channels,
                     obtained.samples, obtained.freq, obtained.channels);

    OPN2_MIDIPlayer *myDevice;
    myDevice = opn2_init(obtained.freq);
    if(myDevice == NULL)
    {
        std::fprintf(stderr, "Failed to init MIDI device!\n");
        return 1;
    }

    #ifdef USE_LEGACY_EMULATOR
    opn2_setNumChips(myDevice, 8);
    #else
    opn2_setNumCards(myDevice, 3);
    #endif

    std::printf("[%s emulator in use]\n", opn2_emulatorName());

    opn2_setLogarithmicVolumes(myDevice, 0);
    opn2_setVolumeRangeModel(myDevice, OPNMIDI_VolumeModel_Generic);

    int arg = 1;
    for(arg = 1; arg < argc; arg++)
    {
        if(!std::strcmp("-nl", argv[arg]))
            opn2_setLoopEnabled(myDevice, 0);
        else if(!std::strcmp("-s", argv[arg]))
            opn2_setScaleModulators(myDevice, 1);
        else if(!std::strcmp("--", argv[arg]))
            break;
        else
            break;
    }

    if(arg > argc - 2)
    {
        std::fprintf(stderr, "Missing paths!\n");
        return 2;
    }

    if(opn2_openBankFile(myDevice, argv[arg]) != 0)
    {
        std::fprintf(stderr, "%s\n", opn2_errorString());
        return 2;
    }

    if(opn2_openFile(myDevice, argv[arg + 1]) != 0)
    {
        std::fprintf(stderr, "%s\n", opn2_errorString());
        return 2;
    }

    SDL_PauseAudio(0);

    signal(SIGINT,  &handle_signal);
    printf("Playing of %s...\n\nPress Ctrl+C to abort", argv[2]);
    fflush(stdout);

    #ifdef DEBUG_DUMP_RAW_STREAM
    passwd* pw = getpwuid(getuid());
    std::string path(pw->pw_dir);
    path += "/opnOutput.raw";
    FILE *rawOutput = fopen(path.c_str(), "wb");
    #endif

    //int16_t buff[204800];
    std::vector<int16_t> buff;
    buff.resize(obtained.samples);
    while(g_playing)
    {
        size_t got = (size_t)opn2_play(myDevice, obtained.samples, buff.data());
        if(got <= 0)
            break;
        #ifdef DEBUG_DUMP_RAW_STREAM
        fwrite(buff.data(), 1, sizeof(int16_t) * buff.size(), rawOutput);
        #endif

        AudioBuffer_lock.Lock();
        size_t pos = g_audioBuffer.size();
        g_audioBuffer.resize(pos + got);
        for(size_t p = 0; p < got; ++p)
            g_audioBuffer[pos + p] = buff[p];
        AudioBuffer_lock.Unlock();

        const SDL_AudioSpec &spec_ = obtained;
        while(g_audioBuffer.size() > spec_.samples + (spec_.freq * 2) * OurHeadRoomLength)
            SDL_Delay(1);
    }

    #ifdef DEBUG_DUMP_RAW_STREAM
    fclose(rawOutput);
    #endif

    opn2_close(myDevice);
    SDL_CloseAudio();

    printf("Bye!\n\n");
    fflush(stdout);

    return 0;
}
