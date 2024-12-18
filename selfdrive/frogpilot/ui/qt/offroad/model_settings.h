#pragma once

#include <set>

#include "selfdrive/frogpilot/ui/qt/offroad/frogpilot_settings.h"

class FrogPilotModelPanel : public FrogPilotListWidget {
  Q_OBJECT

public:
  explicit FrogPilotModelPanel(FrogPilotSettingsWindow *parent);

signals:
  void openParentToggle();
  void openSubParentToggle();

protected:
  void showEvent(QShowEvent *event) override;

private:
  void hideSubToggles();
  void hideToggles();
  void showToggles(const std::set<QString> &keys);
  void updateState(const UIState &s);

  ButtonControl *deleteModelBtn;

  FrogPilotSettingsWindow *parent;

  QDir modelDir{"/data/models/"};

  QJsonObject frogpilotToggleLevels;

  bool modelRandomizerOpen;
  bool started;

  int tuningLevel;

  std::map<QString, AbstractControl*> toggles;

  std::set<QString> modelRandomizerKeys = {"ManageBlacklistedModels", "ResetScores", "ReviewScores"};
};
