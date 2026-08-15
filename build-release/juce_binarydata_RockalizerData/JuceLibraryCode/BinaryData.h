/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   knob_v1_png;
    const int            knob_v1_pngSize = 1680678;

    extern const char*   plugin_background_png;
    const int            plugin_background_pngSize = 1581398;

    extern const char*   rockalizer_logo_v1_png;
    const int            rockalizer_logo_v1_pngSize = 727186;

    extern const char*   logo_tape_v3_png;
    const int            logo_tape_v3_pngSize = 57811;

    extern const char*   logo_chorus_v2_png;
    const int            logo_chorus_v2_pngSize = 52626;

    extern const char*   logo_echo_v2_png;
    const int            logo_echo_v2_pngSize = 50787;

    extern const char*   logo_spring_v2_png;
    const int            logo_spring_v2_pngSize = 63506;

    extern const char*   pedal_tape_v3_png;
    const int            pedal_tape_v3_pngSize = 2658119;

    extern const char*   pedal_chorus_v2_png;
    const int            pedal_chorus_v2_pngSize = 2941420;

    extern const char*   pedal_echo_v2_png;
    const int            pedal_echo_v2_pngSize = 2406074;

    extern const char*   pedal_spring_v2_png;
    const int            pedal_spring_v2_pngSize = 2513763;

    extern const char*   spring_gbsr_clean_wav;
    const int            spring_gbsr_clean_wavSize = 1890798;

    extern const char*   spring_deluxe_clean_wav;
    const int            spring_deluxe_clean_wavSize = 1892352;

    extern const char*   spring_space_clean_wav;
    const int            spring_space_clean_wavSize = 1107948;

    extern const char*   spring_9100_clean_wav;
    const int            spring_9100_clean_wavSize = 2330058;

    extern const char*   spring_echomixer_clean_wav;
    const int            spring_echomixer_clean_wavSize = 1245774;

    extern const char*   spring_schaller_clean_wav;
    const int            spring_schaller_clean_wavSize = 2578800;

    extern const char*   spring_pioneer_clean_wav;
    const int            spring_pioneer_clean_wavSize = 468726;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 18;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
