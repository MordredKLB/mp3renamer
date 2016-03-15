/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  ALBUMPANEL                       1       /* callback function: MusicBrainzPanelCB */
#define  ALBUMPANEL_USEARTIST             2       /* control type: radioButton, callback function: (none) */
#define  ALBUMPANEL_ARTIST                3       /* control type: string, callback function: ValChangeCB */
#define  ALBUMPANEL_USEALBUM              4       /* control type: radioButton, callback function: (none) */
#define  ALBUMPANEL_ALBUM                 5       /* control type: string, callback function: ValChangeCB */
#define  ALBUMPANEL_USEYEAR               6       /* control type: radioButton, callback function: (none) */
#define  ALBUMPANEL_YEAR                  7       /* control type: string, callback function: ValChangeCB */
#define  ALBUMPANEL_USETYPE               8       /* control type: radioButton, callback function: (none) */
#define  ALBUMPANEL_TYPE                  9       /* control type: ring, callback function: ValChangeCB */
#define  ALBUMPANEL_USESTATUS             10      /* control type: radioButton, callback function: (none) */
#define  ALBUMPANEL_STATUS                11      /* control type: ring, callback function: ValChangeCB */
#define  ALBUMPANEL_USETRACKS             12      /* control type: radioButton, callback function: (none) */
#define  ALBUMPANEL_TRACKS                13      /* control type: string, callback function: ValChangeCB */
#define  ALBUMPANEL_USEDISCS              14      /* control type: radioButton, callback function: (none) */
#define  ALBUMPANEL_DISCS                 15      /* control type: string, callback function: ValChangeCB */
#define  ALBUMPANEL_USECOUNTRY            16      /* control type: radioButton, callback function: (none) */
#define  ALBUMPANEL_COUNTRY               17      /* control type: ring, callback function: ValChangeCB */
#define  ALBUMPANEL_USEFORMAT             18      /* control type: radioButton, callback function: (none) */
#define  ALBUMPANEL_FORMAT                19      /* control type: ring, callback function: ValChangeCB */
#define  ALBUMPANEL_MANUALSEARCH          20      /* control type: command, callback function: RefreshReleasesCB */
#define  ALBUMPANEL_DELETEXMLBUTTON       21      /* control type: command, callback function: DeleteXMLFilesCB */
#define  ALBUMPANEL_REFRESHRELEASES       22      /* control type: command, callback function: RefreshReleasesCB */
#define  ALBUMPANEL_USEFUZZYSEARCH        23      /* control type: radioButton, callback function: (none) */
#define  ALBUMPANEL_TRACKTREE             24      /* control type: tree, callback function: (none) */
#define  ALBUMPANEL_ALBUMTREE             25      /* control type: tree, callback function: AlbumTreeCB */
#define  ALBUMPANEL_NEXT                  26      /* control type: command, callback function: OffsetCB */
#define  ALBUMPANEL_PREV                  27      /* control type: command, callback function: OffsetCB */
#define  ALBUMPANEL_OKBUTTON              28      /* control type: command, callback function: AlbumPanelOKCB */
#define  ALBUMPANEL_CANCELBUTTON          29      /* control type: command, callback function: CancelCB */
#define  ALBUMPANEL_TEXTBOX               30      /* control type: textBox, callback function: (none) */
#define  ALBUMPANEL_PROGRESSBAR           31      /* control type: scale, callback function: (none) */
#define  ALBUMPANEL_TEXTMSG               32      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_TXTMESSAGE            33      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_TEXTMSG_2             34      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_DECORATION            35      /* control type: deco, callback function: (none) */
#define  ALBUMPANEL_TEXTMSG_3             36      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_TEXTMSG_4             37      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_TEXTMSG_5             38      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_TXTCATALOG            39      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_TXTASIN               40      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_TEXTMSG_6             41      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_TXTBARCODE            42      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_TXTTYPE               43      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_TEXTMSG_7             44      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_TXTLABEL              45      /* control type: textMsg, callback function: (none) */
#define  ALBUMPANEL_ERROR_ICON            46      /* control type: picture, callback function: (none) */

#define  CDART                            2       /* callback function: FanartPanelCB */
#define  CDART_CDART_5                    2       /* control type: pictButton, callback function: SelectCB */
#define  CDART_CDART_6                    3       /* control type: pictButton, callback function: SelectCB */
#define  CDART_CDART_4                    4       /* control type: pictButton, callback function: SelectCB */
#define  CDART_CDART_2                    5       /* control type: pictButton, callback function: SelectCB */
#define  CDART_CDART_3                    6       /* control type: pictButton, callback function: SelectCB */
#define  CDART_CDART_1                    7       /* control type: pictButton, callback function: SelectCB */
#define  CDART_PREVIEWTIMER               8       /* control type: timer, callback function: CDPreviewTimerCB */

#define  FANART                           3       /* callback function: FanartPanelCB */
#define  FANART_OKBUTTON                  2       /* control type: command, callback function: FanartOKCB */
#define  FANART_CANCELBUTTON              3       /* control type: command, callback function: FanartCancelCB */
#define  FANART_HDTEXT                    4       /* control type: textMsg, callback function: (none) */
#define  FANART_HDTEXT_2                  5       /* control type: textMsg, callback function: (none) */
#define  FANART_PROGRESSBAR               6       /* control type: scale, callback function: (none) */
#define  FANART_DECORATION_2              7       /* control type: deco, callback function: (none) */
#define  FANART_DECORATION                8       /* control type: deco, callback function: (none) */
#define  FANART_ARTISTPAGE                9       /* control type: textMsg, callback function: OpenArtistPageCB */
#define  FANART_TEXTMSG                   10      /* control type: textMsg, callback function: OpenArtistPageCB */
#define  FANART_VINYLART                  11      /* control type: radioButton, callback function: VinylArtCB */
#define  FANART_MULTIPLEDISCS             12      /* control type: radioButton, callback function: (none) */

#define  HDLOGO                           4       /* callback function: FanartPanelCB */
#define  HDLOGO_HDLOGO_1                  2       /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_HDLOGO_2                  3       /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_HDLOGO_3                  4       /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_HDLOGO_4                  5       /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_HDLOGO_5                  6       /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_HDLOGO_6                  7       /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_HDLOGO_10                 8       /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_HDLOGO_11                 9       /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_HDLOGO_12                 10      /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_HDLOGO_7                  11      /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_HDLOGO_8                  12      /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_HDLOGO_9                  13      /* control type: pictButton, callback function: SelectCB */
#define  HDLOGO_PREVIEWTIMER              14      /* control type: timer, callback function: PreviewTimerCB */

#define  HDPREVIEW                        5
#define  HDPREVIEW_TIMER                  2       /* control type: timer, callback function: PreviewDisplayTimerCB */
#define  HDPREVIEW_PREVIEW                3       /* control type: picture, callback function: ColorCB */
#define  HDPREVIEW_BLACK                  4       /* control type: picture, callback function: ColorCB */
#define  HDPREVIEW_RED                    5       /* control type: picture, callback function: ColorCB */
#define  HDPREVIEW_TEAL                   6       /* control type: picture, callback function: ColorCB */
#define  HDPREVIEW_WHITE                  7       /* control type: picture, callback function: ColorCB */

#define  OPTIONS                          6       /* callback function: OptionsPanelCB */
#define  OPTIONS_AUTOGETTAG               2       /* control type: radioButton, callback function: (none) */
#define  OPTIONS_AUTOREVERT               3       /* control type: radioButton, callback function: (none) */
#define  OPTIONS_SHOWLENGTH               4       /* control type: radioButton, callback function: ShowLengthCB */
#define  OPTIONS_SHOWREPLAYGAIN           5       /* control type: radioButton, callback function: (none) */
#define  OPTIONS_EXTENSIONS               6       /* control type: radioButton, callback function: (none) */
#define  OPTIONS_REPLACEAPOSTROPHE        7       /* control type: radioButton, callback function: (none) */
#define  OPTIONS_IGNOREDISC1OF1           8       /* control type: radioButton, callback function: (none) */
#define  OPTIONS_SKIPALBUMARTIST          9       /* control type: radioButton, callback function: (none) */
#define  OPTIONS_POPULATEALBUMORDER       10      /* control type: radioButton, callback function: (none) */
#define  OPTIONS_RENAMEFOLDER             11      /* control type: radioButton, callback function: (none) */
#define  OPTIONS_CHARACTER_1              12      /* control type: string, callback function: CharReplaceCB */
#define  OPTIONS_CHARACTER_2              13      /* control type: string, callback function: CharReplaceCB */
#define  OPTIONS_CHARACTER_3              14      /* control type: string, callback function: CharReplaceCB */
#define  OPTIONS_CHARACTER_4              15      /* control type: string, callback function: CharReplaceCB */
#define  OPTIONS_CHARACTER_5              16      /* control type: string, callback function: CharReplaceCB */
#define  OPTIONS_CHARACTER_6              17      /* control type: string, callback function: CharReplaceCB */
#define  OPTIONS_CHARACTER_7              18      /* control type: string, callback function: CharReplaceCB */
#define  OPTIONS_CHARACTER_8              19      /* control type: string, callback function: CharReplaceCB */
#define  OPTIONS_CHARACTER_9              20      /* control type: string, callback function: CharReplaceCB */
#define  OPTIONS_ALBUMARTPATH             21      /* control type: string, callback function: (none) */
#define  OPTIONS_FANART_APIKEY            22      /* control type: string, callback function: (none) */
#define  OPTIONS_HDLOGOPATH               23      /* control type: string, callback function: (none) */
#define  OPTIONS_PATH                     24      /* control type: string, callback function: (none) */
#define  OPTIONS_OKBUTTON                 25      /* control type: command, callback function: OptionsOKCB */
#define  OPTIONS_QUITBUTTON               26      /* control type: command, callback function: StartDirCancelCB */
#define  OPTIONS_HDLOGODIR                27      /* control type: command, callback function: HDLogoDirCB */
#define  OPTIONS_ARTDOWNLOADERPATH        28      /* control type: command, callback function: AlbumArtDownloaderSearchCB */
#define  OPTIONS_DIRSEARCH                29      /* control type: command, callback function: InitialDirCB */
#define  OPTIONS_DECORATION               30      /* control type: deco, callback function: (none) */
#define  OPTIONS_TEXTMSG                  31      /* control type: textMsg, callback function: (none) */
#define  OPTIONS_DECORATION_2             32      /* control type: deco, callback function: (none) */
#define  OPTIONS_TEXTMSG_2                33      /* control type: textMsg, callback function: (none) */

#define  PANEL                            7       /* callback function: PanelCB */
#define  PANEL_HSPLITTER                  2       /* control type: splitter, callback function: (none) */
#define  PANEL_BrowseButton               3       /* control type: command, callback function: BrowseCB */
#define  PANEL_ARTIST                     4       /* control type: string, callback function: ArtistCB */
#define  PANEL_ALBUM                      5       /* control type: string, callback function: AlbumCB */
#define  PANEL_FORMATSTRING               6       /* control type: table, callback function: (none) */
#define  PANEL_REMOVESTRING               7       /* control type: string, callback function: (none) */
#define  PANEL_FIXNUMCHECKBOX             8       /* control type: radioButton, callback function: (none) */
#define  PANEL_REMOVEARTIST               9       /* control type: radioButton, callback function: (none) */
#define  PANEL_UNDERSCORECHECKBOX         10      /* control type: radioButton, callback function: RemoveUnderscoresCB */
#define  PANEL_REMOVEALBUM                11      /* control type: radioButton, callback function: (none) */
#define  PANEL_STRIPYEAR                  12      /* control type: radioButton, callback function: (none) */
#define  PANEL_REMOVENUMSTART             13      /* control type: numeric, callback function: (none) */
#define  PANEL_REMOVENUM                  14      /* control type: numeric, callback function: (none) */
#define  PANEL_CAPITALIZE                 15      /* control type: radioButton, callback function: CapitalizeCB */
#define  PANEL_SMARTCAPS                  16      /* control type: radioButton, callback function: SmartCapCB */
#define  PANEL_SMARTCAPID3                17      /* control type: radioButton, callback function: (none) */
#define  PANEL_COMMANDBUTTON              18      /* control type: command, callback function: CapitalizeTitlesCB */
#define  PANEL_UPDATETITLE                19      /* control type: radioButton, callback function: (none) */
#define  PANEL_DOID3V1                    20      /* control type: radioButton, callback function: ID3v1CB */
#define  PANEL_SHOWTRACKARTISTS           21      /* control type: radioButton, callback function: ShowTrackArtistCB */
#define  PANEL_UPDATETRACKNUM             22      /* control type: radioButton, callback function: (none) */
#define  PANEL_ID3V1COMMENTS              23      /* control type: radioButton, callback function: (none) */
#define  PANEL_GUESSTITLES                24      /* control type: command, callback function: GuessTitlesCB */
#define  PANEL_RENUMBERBUTTON             25      /* control type: command, callback function: RenumberTracks */
#define  PANEL_TAB                        26      /* control type: tab, callback function: TabCB */
#define  PANEL_TREE                       27      /* control type: tree, callback function: TreeCB */
#define  PANEL_GetMusicBrainzData         28      /* control type: command, callback function: GetMetaDataCB */
#define  PANEL_GETID3BUTTON               29      /* control type: command, callback function: GetID3Tag */
#define  PANEL_ID3BUTTON                  30      /* control type: command, callback function: SetID3Tag */
#define  PANEL_PREVIEWBUTTON              31      /* control type: command, callback function: PreviewNames */
#define  PANEL_REVERTBUTTON               32      /* control type: command, callback function: RevertNames */
#define  PANEL_SETBUTTON                  33      /* control type: command, callback function: SetNamesCB */
#define  PANEL_FANART                     34      /* control type: command, callback function: RetrieveFanart */
#define  PANEL_RENAMEFOLDER               35      /* control type: command, callback function: RenameFolderCB */
#define  PANEL_DLARTWORKBUTTON            36      /* control type: command, callback function: DownloadArtworkCB */
#define  PANEL_IMAGERING                  37      /* control type: ring, callback function: ImageRingCB */
#define  PANEL_QUITBUTTON                 38      /* control type: command, callback function: QuitCallback */
#define  PANEL_EXPLORECURRENT             39      /* control type: command, callback function: ExploreCurrentCB */
#define  PANEL_UPDATEARTIST               40      /* control type: radioButton, callback function: (none) */
#define  PANEL_UPDATEALBUM                41      /* control type: radioButton, callback function: (none) */
#define  PANEL_TRACKNUMLED                42      /* control type: LED, callback function: (none) */
#define  PANEL_DECORATION_2               43      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_3               44      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION                 45      /* control type: deco, callback function: (none) */
#define  PANEL_TEXTMSG                    46      /* control type: textMsg, callback function: (none) */
#define  PANEL_ALBUMLED                   47      /* control type: LED, callback function: (none) */
#define  PANEL_TABVALS2                   48      /* control type: LED, callback function: tabLEDCallback */
#define  PANEL_TABVALS                    49      /* control type: LED, callback function: tabLEDCallback */
#define  PANEL_ARTISTLED                  50      /* control type: LED, callback function: (none) */
#define  PANEL_PICTURE_5                  51      /* control type: picture, callback function: (none) */
#define  PANEL_TRACKNUM                   52      /* control type: string, callback function: (none) */
#define  PANEL_TEXTMSG_2                  53      /* control type: textMsg, callback function: (none) */
#define  PANEL_TEXTMSG_3                  54      /* control type: textMsg, callback function: (none) */
#define  PANEL_VSPLITTER                  55      /* control type: splitter, callback function: (none) */
#define  PANEL_LOG                        56      /* control type: textBox, callback function: (none) */
#define  PANEL_TEXTMSG_4                  57      /* control type: textMsg, callback function: (none) */

#define  PROGRESS                         8
#define  PROGRESS_PROGRESSBAR             2       /* control type: scale, callback function: (none) */

     /* tab page panel controls */
#define  TAB1_GENRE                       2       /* control type: table, callback function: TableComboCB */
#define  TAB1_COMMENT                     3       /* control type: textBox, callback function: TagCB */
#define  TAB1_YEAR                        4       /* control type: string, callback function: TagCB */
#define  TAB1_DISCNUM                     5       /* control type: string, callback function: TagCB */
#define  TAB1_COMPOSER                    6       /* control type: string, callback function: TagCB */
#define  TAB1_PUBLISHER                   7       /* control type: string, callback function: TagCB */
#define  TAB1_EDITION                     8       /* control type: string, callback function: TagCB */
#define  TAB1_COUNTRY                     9       /* control type: string, callback function: TagCB */
#define  TAB1_UKBUTTON                    10      /* control type: command, callback function: AutoArtistCountryCB */
#define  TAB1_USBUTTON                    11      /* control type: command, callback function: AutoArtistCountryCB */
#define  TAB1_RELTYPE                     12      /* control type: table, callback function: TableComboCB */
#define  TAB1_ALBUMARTIST                 13      /* control type: string, callback function: TagCB */
#define  TAB1_VABUTTON                    14      /* control type: command, callback function: AutoAlbumArtistCB */
#define  TAB1_SOUNDTRACKBUTTON            15      /* control type: command, callback function: AutoAlbumArtistCB */
#define  TAB1_ARTISTFILTER                16      /* control type: string, callback function: TagCB */
#define  TAB1_PERFORMERSORTORDER          17      /* control type: string, callback function: TagCB */
#define  TAB1_GUESSBUTTON                 18      /* control type: command, callback function: GuessSortOrderCB */
#define  TAB1_ALBUMGAIN                   19      /* control type: string, callback function: TagCB */
#define  TAB1_ALBUMSORTORDER              20      /* control type: string, callback function: TagCB */
#define  TAB1_UPDATEGENRE                 21      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATECOMMENT               22      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATEYEAR                  23      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATECOMPOSER              24      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATEPUBLISHER             25      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATEEDITION               26      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATECOUNTRY               27      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATERELTYPE               28      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATEALBUMARTIST           29      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATEARTISTFILTER          30      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATEPERFSORT              31      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATEALBUMGAIN             32      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATEDISCNUM               33      /* control type: radioButton, callback function: (none) */
#define  TAB1_UPDATEALBUMSORT             34      /* control type: radioButton, callback function: (none) */
#define  TAB1_GENRELED                    35      /* control type: LED, callback function: (none) */
#define  TAB1_COMMENTLED                  36      /* control type: LED, callback function: (none) */
#define  TAB1_YEARLED                     37      /* control type: LED, callback function: (none) */
#define  TAB1_RELTYPELED                  38      /* control type: LED, callback function: (none) */
#define  TAB1_ALBUMARTISTLED              39      /* control type: LED, callback function: (none) */
#define  TAB1_EDITIONLED                  40      /* control type: LED, callback function: (none) */
#define  TAB1_COUNTRYLED                  41      /* control type: LED, callback function: (none) */
#define  TAB1_PERFSORTLED                 42      /* control type: LED, callback function: (none) */
#define  TAB1_PUBLISHERLED                43      /* control type: LED, callback function: (none) */
#define  TAB1_ALBUMGAINLED                44      /* control type: LED, callback function: (none) */
#define  TAB1_ARTISTFILTERLED             45      /* control type: LED, callback function: (none) */
#define  TAB1_DISCNUMLED                  46      /* control type: LED, callback function: (none) */
#define  TAB1_ALBUMSORTLED                47      /* control type: LED, callback function: (none) */
#define  TAB1_COMPOSERLED                 48      /* control type: LED, callback function: (none) */
#define  TAB1_COUNTRYERROR                49      /* control type: textMsg, callback function: (none) */

     /* tab page panel controls */
#define  TAB2_ORIGARTIST                  2       /* control type: string, callback function: TagCB */
#define  TAB2_URL                         3       /* control type: string, callback function: TagCB */
#define  TAB2_COPYRIGHT                   4       /* control type: string, callback function: TagCB */
#define  TAB2_ENCODED                     5       /* control type: string, callback function: TagCB */
#define  TAB2_ORIGARTISTLED               6       /* control type: LED, callback function: (none) */
#define  TAB2_UPDATEORIGARTIST            7       /* control type: radioButton, callback function: (none) */
#define  TAB2_ARTWORKLED                  8       /* control type: LED, callback function: (none) */
#define  TAB2_URLLED                      9       /* control type: LED, callback function: (none) */
#define  TAB2_CLEARARTWORK                10      /* control type: radioButton, callback function: ClearArtworkCB */
#define  TAB2_UPDATEARTWORK               11      /* control type: radioButton, callback function: (none) */
#define  TAB2_UPDATEURL                   12      /* control type: radioButton, callback function: (none) */
#define  TAB2_ARTWORK                     13      /* control type: picture, callback function: (none) */
#define  TAB2_IMAGECORRUPTEDMSG           14      /* control type: textMsg, callback function: (none) */
#define  TAB2_IMAGESIZEMSG                15      /* control type: textMsg, callback function: (none) */
#define  TAB2_LOADARTWORKBUTTON           16      /* control type: command, callback function: LoadArtworkCB */
#define  TAB2_TEXTMSG                     17      /* control type: textMsg, callback function: (none) */
#define  TAB2_UPDATECOPYRIGHT             18      /* control type: radioButton, callback function: (none) */
#define  TAB2_COPYRIGHTLED                19      /* control type: LED, callback function: (none) */
#define  TAB2_UPDATEENCODED               20      /* control type: radioButton, callback function: (none) */
#define  TAB2_ENCODEDLED                  21      /* control type: LED, callback function: (none) */

     /* tab page panel controls */
#define  TAB3_ARTISTMBID                  2       /* control type: string, callback function: TagCB */
#define  TAB3_REID                        3       /* control type: string, callback function: TagCB */
#define  TAB3_EXTENDEDTAGS                4       /* control type: tree, callback function: ExtendedTreeCB */
#define  TAB3_MUSICBRAINZ_ARTIST          5       /* control type: pictButton, callback function: LaunchMBCB */
#define  TAB3_FANARTBUTTON                6       /* control type: pictButton, callback function: OpenArtistPageCB */
#define  TAB3_MUSICBRAINZ_RELEASE         7       /* control type: pictButton, callback function: LaunchMBCB */
#define  TAB3_UPDATEMBID                  8       /* control type: radioButton, callback function: (none) */
#define  TAB3_UPDATEREID                  9       /* control type: radioButton, callback function: (none) */
#define  TAB3_ADDFIELD                    10      /* control type: command, callback function: AddFieldCB */


     /* Control Arrays: */

#define  CHARACTER_ARRAY                  1

     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_MENU                     2
#define  MENUBAR_MENU_SETTINGS            3       /* callback function: OptionsCB */

#define  PMENU                            2
#define  PMENU_MENU1                      2       /* callback function: DownloadFanartCB */
#define  PMENU_MENU1_ITEM1                3       /* callback function: DownloadFanartCB */


     /* Callback Prototypes: */

int  CVICALLBACK AddFieldCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AlbumArtDownloaderSearchCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AlbumCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AlbumPanelOKCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AlbumTreeCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ArtistCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AutoAlbumArtistCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AutoArtistCountryCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BrowseCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CancelCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CapitalizeCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CapitalizeTitlesCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CDPreviewTimerCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CharReplaceCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ClearArtworkCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ColorCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DeleteXMLFilesCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DownloadArtworkCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK DownloadFanartCB(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK ExploreCurrentCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ExtendedTreeCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FanartCancelCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FanartOKCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FanartPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GetID3Tag(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GetMetaDataCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GuessSortOrderCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GuessTitlesCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK HDLogoDirCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ID3v1CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ImageRingCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK InitialDirCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK LaunchMBCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK LoadArtworkCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MusicBrainzPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OffsetCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OpenArtistPageCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK OptionsCB(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK OptionsOKCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OptionsPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PreviewDisplayTimerCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PreviewNames(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PreviewTimerCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RefreshReleasesCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RemoveUnderscoresCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RenameFolderCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RenumberTracks(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RetrieveFanart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RevertNames(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SelectCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SetID3Tag(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SetNamesCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ShowLengthCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ShowTrackArtistCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SmartCapCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK StartDirCancelCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TabCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TableComboCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK tabLEDCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TagCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TreeCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ValChangeCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK VinylArtCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
