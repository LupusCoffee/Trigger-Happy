#pragma once

// Central place for editor UI sizing used by customizations.
namespace GP4EditorUI
{
	// Width for enum pickers (Event/Speaker, both per-node and overrides)
	inline constexpr float EnumPickerWidth = 220.f;

	// Duration column width and numeric field width
	inline constexpr float DurationColumnWidth = 220.f;
	inline constexpr float DurationNumericWidth = 120.f;
	// Gap between the checkbox label and the numeric entry in duration widgets
	inline constexpr float DurationGapPadding = 6.f;

	// Sound asset picker width
	inline constexpr float SoundPickerWidth = 200.f;

	// Minimum width used for the Line sort header (visual alignment)
	inline constexpr float SortHeaderLineMinWidth = 260.f;

	// Indent used for showing the custom value below an enum when set to Custom
	inline constexpr float CustomLeftIndent = 2.f;

	// Gameplay tag UI sizing
	inline constexpr float TagComboMinWidth = 260.f;
	inline constexpr float TagPickerMenuWidth = 400.f;
	inline constexpr float TagPickerMenuHeight = 450.f;
}
