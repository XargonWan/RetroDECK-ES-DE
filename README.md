# EmulationStation Desktop Edition (ES-DE)

EmulationStation Desktop Edition is a frontend for browsing and launching games from your multi-platform game collection.

Website:\
[https://es-de.org](https://es-de.org)

Discord:\
[https://discord.gg/EVVX4DqWAP](https://discord.gg/EVVX4DqWAP)

The goal of this edition is to make the software as easy as possible to install and use as a standalone application, and to support the major desktop operating systems, i.e. Windows, macOS and Unix/Linux. Since it's intended to be used as a desktop application rather than as the primary interface for the computer, there are no plans to provide system administration utilities or control over emulator settings from within ES-DE.

It comes preconfigured for use with [RetroArch](https://www.retroarch.com) and a large selection of standalone emulators. It's also fully customizable so you can easily expand it to include emulators or applications not covered by the bundled configuration.

You can find the complete list of supported systems and emulators [here](USERGUIDE.md#supported-game-systems).

Two themes (Slate and Modern) are bundled with the application, and additional themes can be found on the [official themes list](https://gitlab.com/es-de/themes/themes-list) or be installed using the built-in theme downloader.

## Download

Visit https://es-de.org/ to download the latest ES-DE release or go to the [package registry](https://gitlab.com/es-de/emulationstation-de/-/packages) where you can also find most previous releases.

If you're using a Raspberry Pi or if you run FreeBSD, NetBSD or OpenBSD then you need to compile from source code as no prebuilt packages are provided for these platforms. A detailed build guide is available in [INSTALL.md](INSTALL.md).

## Additional information

[FAQ.md](FAQ.md) -  Frequently Asked Questions

[USERGUIDE.md](USERGUIDE.md) / [USERGUIDE-DEV.md](USERGUIDE-DEV.md) - Comprehensive guide and reference for all application settings

[INSTALL.md](INSTALL.md) / [INSTALL-DEV.md](INSTALL-DEV.md) - Building from source code and advanced configuration topics

[THEMES.md](THEMES.md) / [THEMES-DEV.md](THEMES-DEV.md) - Guide and reference for theme development

[CHANGELOG.md](CHANGELOG.md) - Detailed list of changes for all past releases and the in-development version

[ROADMAP.md](ROADMAP.md) - List of major features planned to be added in the future

[CONTRIBUTING.md](CONTRIBUTING.md) - Information on how to contribute to the project

[CREDITS.md](CREDITS.md) - An attempt to credit the individuals and projects which made ES-DE possible

## Some feature highlights

Here are some highlights, displayed using the default Slate theme.

![alt text](images/es-de_system_view.png "ES-DE System View")
_The **System view**, which is the default starting point for the application, it's here that you browse through your game systems._

![alt text](images/es-de_gamelist_view.png "ES-DE Gamelist View")
_The **Gamelist view**, it's here that you browse the actual games per system._

![alt text](images/es-de_folder_support.png "ES-DE Folder Support")
_Another example of the gamelist view, displaying advanced folder support. You can scrape folders for game info and game media, sort folders as you would files, mark them as favorites etc._

![alt text](images/es-de_custom_collections.png "ES-DE Custom Collections")
_Games can be grouped into your own custom collections, in this example they're defined as genres._

![alt text](images/es-de_scraper_running.png "ES-DE Scraper Running")
_This is a view of the built-in scraper which downloads game info and game media from either [screenscraper.fr](https://screenscraper.fr) or [thegamesdb.net](https://thegamesdb.net). It's possible to scrape a single game, or to run the multi-scraper which can scrape a complete game system or even your entire collection._

![alt text](images/es-de_scraper_settings.png "ES-DE Scraper Settings")
_There are many settings for the scraper including options to define which type of info and media to download. The above screenshot shows only a portion of these settings._

![alt text](images/es-de_metadata_editor.png "ES-DE Metadata Editor")
_In addition to the scraper there is a fully-featured metadata editor that can be used to modify information on a per-game basis._

![alt text](images/es-de_screensaver.png "ES-DE Screensaver")
_There are four built-in screensavers, including a slideshow and a video screensaver that display random games from your collection._

![alt text](images/es-de_ui_theme_support.png "ES-DE Theme Support")
_ES-DE is fully themeable, in case you prefer another look than what the default theme Slate offers. The screenshot above shows the Modern theme that is also bundled with the application._
