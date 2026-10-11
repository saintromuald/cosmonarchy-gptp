#pragma once

#include "BW/ui.h"
#include "BW/enumerations/GluIndex.h"

class GluChatContext {
public:
    GluChatContext() : m_labels({
        GluChatControls::GamePasswordValue,
        GluChatControls::GameNameValue,
        GluChatControls::MapNameValue,
        GluChatControls::GameTypeValue,
        GluChatControls::GameModeValue,
        GluChatControls::SubtypeOrReplayValue,
        GluChatControls::MapSizeValue,
    }) {}

    void update_label(GluChatControls control, const char* text);

private:
    BinDlgLabels<GluChatControls> m_labels;
};

void glu_chat();
void glu_chat_race_combobox_prep(BinDlg* dialog, RaceId::Enum selected_Race, u32 slot_id = 0);