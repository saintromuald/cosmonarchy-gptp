#pragma once
#include "BW/ui.h"
#include "glu_chat.h"

class GluJoinCrownlinkContext {
public:
    GluJoinCrownlinkContext()
        : m_labels({
        GluJoinControls::LobbyPasswordLabel,
        GluJoinControls::CrownLinkStatus,
        GluJoinControls::GameTitle,
        GluJoinControls::MapName,
        GluJoinControls::GameType,
        GluJoinControls::Replay,
        GluJoinControls::SubType,
        GluJoinControls::MapSize,
        }),
        m_binfile("rez\\glujoincrownlink.bin") {
    }

    BinDlg* get_dialog() { return m_binfile.bin_dlg(); };

    void update_crownlink_status();
    void load_selected_entry(BinDlg* dialog);
    void update_listbox(BinDlg* dialog);

    void on_create(BinDlg* dialog, DialogEvent* event);
    u32 on_activate(BinDlg* dialog, DialogEvent* event);
    void on_init_children(BinDlg* dialog, DialogEvent* event);

private:
    void update_label(GluJoinControls control, const char* text);
    void update_password(const char* password);
    void clear_lobby_labels();
    void update_lobby_labels(GameData& game_data);
    u32 selected_adfile_index(BinDlg* dialog) const;
    std::string p2p_status_string(AdFileEx& adfile) const;
    std::string format_listbox_line(AdFileEx& adfile) const;

private:
    BinDlgLabels<GluJoinControls> m_labels;
    BinFile m_binfile;
    GameData m_gamedata{};
    u8 m_ellipsis_counter{0};
    std::vector<u32> m_adfile_indexes;
    std::list<std::string> m_listbox_entries;
};

void glu_main();
void glu_login();
void glu_provider_select(bool from_crownlink);
void glu_join();
void glu_join_crownlink();
void glu_singleplayer();
void glu_options();

void glu_main_switch();

namespace hooks {
void inject_custom_glues();
}