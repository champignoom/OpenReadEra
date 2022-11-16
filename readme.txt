The OpenReadEra Project

1. About.

The OpenReadEra Project consists of four independent programs: EraPDF, EraEPUB, EraDjVu, EraMOBI and EraComic.

- EraPDF: parsing and rendering of PDF docs.
- EraEPUB: parsing and rendering of EPUB, FB2, FB3, DOC, DOCX, ODT, RTF, CHM and TXT docs.
- EraDjVu: parsing and rendering of DJVU docs.
- EraMOBI: parsing and rendering of MOBI, AZW and AZW3 docs.
- EraComic: parsing and rendering of CBR and CBZ docs.

The programs of The OpenReadEra Project are separate, completely independent programs, they don't using static linking and don't combined into single executable file. They run in different processes of the operating system, don't use dynamic linking, don't use shared memory, and exchange simple types of data only through interprocess communication mechanisms, such as command line arguments, named pipes, and sockets.

2. Build.

To perform build, you need the Android Studio, Android SDK and NDK v20.1. Create new standard Android project, place The OpenReadEra Project sources directory to the "cpp" project sources directory and configure Gradle build. You can build all four programs at once or build only some of them by listing desired programs names in "targets" list.

Sample Gradle configuration for building all four programs at once:

android.defaultConfig.externalNativeBuild.ndkBuild {
    targets 'eradjvu', 'eraepub', 'eramobi', 'erapdf', 'eracomic'
    arguments 'APP_STL:=c++_static'
    cFlags '-no-integrated-as'
}
android.externalNativeBuild.ndkBuild {
    path 'src/main/cpp/openreadera/Android.mk'
}

3. Usage.

EraPDF, EraEPUB, EraDjVu and EraMOBI programs initializes operation via CLI and then communicating via named pipes.

Basic principle:
- Create two FIFO pipes.
- Start chosen program binary and pass two created FIFO pipes as CLI arguments.
- Write your requests to first FIFO pipe and read program responses from second FIFO pipe.

4. Licenses.

The programs of The OpenReadEra Project are free software and are distributed under the terms of the open GNU GPL licenses.

You can get acquainted with the copyright notices and licenses of EraPDF, EraEPUB, EraDjVu, EraMOBI and EraComic programs, and other third-party projects wich they based upon, in the corresponding sources directories of those programs and projects.

Origin address, where you can download this The OpenReadEra Project sources: https://readera.org/book-reader/OpenReadEra

5. Third-party

EraPDF is based on MuPDF, The Common CLI viewer interface Project, The FreeType Project, libjpeg-turbo, jbig2dec, OpenJPEG, orecrop.
EraEPUB is based on CoolReader, The Common CLI viewer interface Project, The FreeType Project, Antiword, chmlib, libjpeg, libpng.
EraDjVu is based on DjVuLibre, The Common CLI viewer interface Project, libjpeg-turbo, orecrop.
EraMOBI is based on Libmobi, The Common CLI viewer interface Project, libjpeg, libpng.
EraComic is based on The Common CLI viewer interface Project, libJpeg-turbo, libPng, EasyBmp, libWebp, dmc_unrar, MiniZip, zlib.

6. Contacts.

If you have any questions, you can contact us at: support@readera.org