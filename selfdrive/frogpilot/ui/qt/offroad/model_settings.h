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
  void startDownloadAllModels();
  void updateModelLabels();
  void updateState(const UIState &s);

  ButtonControl *deleteModelBtn;
  ButtonControl *downloadAllModelsBtn;
  ButtonControl *downloadModelBtn;
  ButtonControl *selectModelBtn;

  FrogPilotSettingsWindow *parent;

  Params params;
  Params params_memory{"/dev/shm/params"};
  Params paramsStorage{"/persist/params"};

  QDir modelDir{"/data/models/"};

  QJsonObject frogpilotToggleLevels;

  QList<LabelControl*> labelControls;

  QStringList availableModelNames;
  QStringList availableModels;
  QStringList experimentalModels;

  bool allModelsDownloading;
  bool cancellingDownload;
  bool haveModelsDownloaded;
  bool modelDeleting;
  bool modelDownloading;
  bool modelRandomizer;
  bool modelRandomizerOpen;
  bool modelsDownloaded;
  bool started;

  int tuningLevel;

  std::map<QString, AbstractControl*> toggles;

  std::set<QString> modelRandomizerKeys = {"ManageBlacklistedModels", "ResetScores", "ReviewScores"};
};