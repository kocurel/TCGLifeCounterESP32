#ifndef GAME_SETTINGS_PAGE_H
#define GAME_SETTINGS_PAGE_H

/**
 * @brief Otwiera stronę ustawień rozgrywki (Game Rules).
 * * Pozwala użytkownikowi zmienić:
 * - Życie startowe (20, 30, 40, 50)
 * - Zasady przegranej (śmierć przy 0 HP)
 * - Commander Damage (czy 21 obrażeń zabija)
 * - Szybkość przewijania liczników
 * * Zmiany są zapisywane w NVS dopiero po wybraniu opcji "Accept".
 * "Cancel" odrzuca zmiany.
 */
void GameSettingsPage_enter(void);

#endif  // GAME_SETTINGS_PAGE_H