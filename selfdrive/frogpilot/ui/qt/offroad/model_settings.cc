#include "selfdrive/frogpilot/ui/qt/offroad/model_settings.h"

FrogPilotModelPanel::FrogPilotModelPanel(FrogPilotSettingsWindow *parent) : FrogPilotListWidget(parent), parent(parent) {
  const std::vector<std::tuple<QString, QString, QString, QString>> modelToggles {
    {"AutomaticallyUpdateModels", tr("Automatically Update and Download Models"), tr("Automatically downloads new models and updates them if needed."), ""},

    {"ModelRandomizer", tr("Model Randomizer"), tr("Randomly selects a model each drive and brings up a prompt at the end of the drive to review the model if it's longer than 15 minutes to help find your preferred model."), ""},
    {"ManageBlacklistedModels", tr("Manage Model Blacklist"), tr("Controls which models are blacklisted and won't be used for future drives."), ""},
    {"ResetScores", tr("Reset Model Scores"), tr("Clears the ratings you've given to the driving models."), ""},
    {"ReviewScores", tr("Review Model Scores"), tr("Displays the ratings you've assigned to the driving models."), ""},

    {"DeleteModel", tr("Delete Model"), tr("Removes the selected driving model from your device."), ""},
    {"DownloadModel", tr("Download Model"), tr("Downloads the selected driving model."), ""},
    {"SelectModel", tr("Select Model"), tr("Selects which model openpilot uses to drive."), ""},
  };

  for (const auto &[param, title, desc, icon] : modelToggles) {
    AbstractControl *modelToggle;

    if (param == "ModelRandomizer") {
      FrogPilotParamManageControl *modelRandomizerToggle = new FrogPilotParamManageControl(param, title, desc, icon);
      QObject::connect(modelRandomizerToggle, &FrogPilotParamManageControl::manageButtonClicked, [this]() {
        modelRandomizerOpen = true;
        showToggles(modelRandomizerKeys);
      });
      modelToggle = modelRandomizerToggle;

    } else if (param == "DeleteModel") {
      deleteModelBtn = new FrogPilotButtonsControl(title, desc, {tr("DELETE"), tr("DELETE ALL")});
      QObject::connect(deleteModelBtn, &FrogPilotButtonsControl::buttonClicked, [this](int id) {
        QStringList deletableModels;
        for (const QString &file : modelDir.entryList(QDir::Files)) {
          QString modelName = modelFileToNameMap.value(QFileInfo(file).baseName());
          if (!modelName.isEmpty()) {
            deletableModels.append(modelName);
          }
        }
        deletableModels.removeAll(QString::fromStdString(params.get("ModelName")));
        deletableModels.removeAll(QString::fromStdString(params_default.get("ModelName")));
        deletableModels.sort();

        if (id == 0) {
          QString modelToDelete = MultiOptionDialog::getSelection(tr("Select a driving model to delete"), deletableModels, "", this);
          if (!modelToDelete.isEmpty() && ConfirmationDialog::confirm(tr("Are you sure you want to delete the '%1' model?").arg(modelToDelete), tr("Delete"), this)) {
            QString modelFile = modelFileToNameMap.key(modelToDelete);
            for (const QString &file : modelDir.entryList(QDir::Files)) {
              if (QFileInfo(file).baseName() == modelFile) {
                QFile::remove(modelDir.filePath(file));
                break;
              }
            }
            allModelsDownloaded = false;
          }
        } else if (id == 1) {
          if (ConfirmationDialog::confirm(tr("Are you sure you want to delete all of your downloaded driving models?"), tr("Delete"), this)) {
            for (const QString &file : modelDir.entryList(QDir::Files)) {
              QFile::remove(modelDir.filePath(file));
            }
            allModelsDownloaded = false;
          }
        }
      });
      modelToggle = deleteModelBtn;
    } else if (param == "DownloadModel") {
      downloadModelBtn = new FrogPilotButtonsControl(title, desc, {tr("DOWNLOAD"), tr("DOWNLOAD ALL")});
      QObject::connect(downloadModelBtn, &FrogPilotButtonsControl::buttonClicked, [this](int id) {
        if (id == 0) {
          if (modelDownloading) {
            params_memory.putBool("CancelModelDownload", true);
            params_memory.remove("DownloadAllModels");
            params_memory.remove("ModelToDownload");
          } else {
            QStringList downloadableModels = availableModelNames;
            for (const QString &file : modelDir.entryList(QDir::Files)) {
              downloadableModels.removeAll(modelFileToNameMap.value(QFileInfo(file).baseName()));
            }
            downloadableModels.removeAll(QString::fromStdString(params_default.get("ModelName")));
            downloadableModels.sort();

            QString modelToDownload = MultiOptionDialog::getSelection(tr("Select a driving model to download"), downloadableModels, "", this);
            if (!modelToDownload.isEmpty()) {
              modelDownloading = true;

              params_memory.put("ModelToDownload", modelFileToNameMap.key(modelToDownload).toStdString());
              params_memory.put("ModelDownloadProgress", "Downloading...");

              downloadModelBtn->setValue("Downloading...");
            }
          }
        } else if (id == 1) {
          if (modelDownloading) {
            params_memory.putBool("CancelModelDownload", true);
            params_memory.remove("DownloadAllModels");
          } else {
            allModelsDownloading = true;

            params_memory.putBool("DownloadAllModels", true);
            params_memory.put("ModelDownloadProgress", "Downloading...");

            downloadModelBtn->setValue("Downloading...");
          }
        }
      });
      modelToggle = downloadModelBtn;
    } else if (param == "SelectModel") {
      selectModelBtn = new ButtonControl(title, tr("SELECT"), desc);
      QObject::connect(selectModelBtn, &ButtonControl::clicked, [this]() {
        QStringList selectableModels;
        for (const QString &file : modelDir.entryList(QDir::Files)) {
          QString modelName = modelFileToNameMap.value(QFileInfo(file).baseName());
          if (!modelName.isEmpty()) {
            selectableModels.append(modelName);
          }
        }
        selectableModels.sort();

        QString modelToSelect = MultiOptionDialog::getSelection(tr("Select a driving model to use"), selectableModels, QString::fromStdString(params.get("ModelName")), this);
        if (!modelToSelect.isEmpty()) {
          params.put("Model", modelFileToNameMap.key(modelToSelect).toStdString());
          params.put("ModelName", modelToSelect.toStdString());

          if (started) {
            if (FrogPilotConfirmationDialog::toggle(tr("Reboot required to take effect."), tr("Reboot Now"), this)) {
              Hardware::reboot();
            }
          }
        }
      });
      modelToggle = selectModelBtn;

    } else {
      modelToggle = new ParamControl(param, title, desc, icon);
    }

    addItem(modelToggle);
    toggles[param] = modelToggle;

    if (FrogPilotParamManageControl *frogPilotManageToggle = qobject_cast<FrogPilotParamManageControl*>(modelToggle)) {
      QObject::connect(frogPilotManageToggle, &FrogPilotParamManageControl::manageButtonClicked, this, &FrogPilotModelPanel::openParentToggle);
    }

    QObject::connect(modelToggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });
  }

  QObject::connect(parent, &FrogPilotSettingsWindow::closeParentToggle, this, &FrogPilotModelPanel::hideToggles);
  QObject::connect(parent, &FrogPilotSettingsWindow::closeSubParentToggle, this, &FrogPilotModelPanel::hideSubToggles);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotModelPanel::updateState);
}

void FrogPilotModelPanel::showEvent(QShowEvent *event) {
  frogpilotToggleLevels = parent->frogpilotToggleLevels;
  tuningLevel = parent->tuningLevel;

  availableModels = QString::fromStdString(params.get("AvailableModels")).split(',');
  availableModelNames = QString::fromStdString(params.get("AvailableModelNames")).split(',');

  int size = qMin(availableModels.size(), availableModelNames.size());
  for (int i = 0; i < size; ++i) {
    modelFileToNameMap.insert(availableModels[i], availableModelNames[i]);
  }

  allModelsDownloaded = true;
  for (const QString &model : availableModels) {
    if (!modelDir.exists(model)) {
      allModelsDownloaded = false;
      break;
    }
  }

  hideToggles();
}

void FrogPilotModelPanel::updateState(const UIState &s) {
  if (!isVisible()) return;

  if (allModelsDownloading || modelDownloading) {
    QString progress = QString::fromStdString(params_memory.get("ModelDownloadProgress"));
    bool downloadFailed = progress.contains(QRegularExpression("cancelled|exists|failed|offline", QRegularExpression::CaseInsensitiveOption));

    if (progress != "Downloading...") {
      downloadModelBtn->setValue(progress);
    }

    if (progress == "Downloaded!" && modelDownloading || progress == "All models downloaded!" && allModelsDownloading || downloadFailed) {
      QTimer::singleShot(2000, [=]() {
        if (!modelDownloading) {
          downloadModelBtn->setValue("");
        }
      });

      params_memory.remove("ModelDownloadProgress");

      allModelsDownloaded = progress == "All models downloaded!";
      allModelsDownloading = false;
      modelDownloading = false;
    }
  }

  bool parked = !started || s.scene.parked || s.scene.frogs_go_moo;

  downloadModelBtn->setText(0, modelDownloading ? tr("CANCEL") : tr("DOWNLOAD"));
  downloadModelBtn->setText(1, allModelsDownloading ? tr("CANCEL") : tr("DOWNLOAD"));

  downloadModelBtn->setEnabledButtons(0, !allModelsDownloaded && !allModelsDownloading && s.scene.online && parked);
  downloadModelBtn->setEnabledButtons(1, !allModelsDownloaded && !modelDownloading && s.scene.online && parked);

  downloadModelBtn->setVisibleButton(0, !allModelsDownloading);
  downloadModelBtn->setVisibleButton(1, !modelDownloading);

  started = s.scene.started;
}

void FrogPilotModelPanel::showToggles(const std::set<QString> &keys) {
  setUpdatesEnabled(false);

  for (auto &[key, toggle] : toggles) {
    toggle->setVisible(keys.find(key) != keys.end() && tuningLevel >= frogpilotToggleLevels[key].toDouble());
  }

  setUpdatesEnabled(true);
  update();
}

void FrogPilotModelPanel::hideToggles() {
  setUpdatesEnabled(false);

  modelRandomizerOpen = false;

  for (auto &[key, toggle] : toggles) {
    bool subToggles = modelRandomizerKeys.find(key) != modelRandomizerKeys.end();

    toggle->setVisible(!subToggles && tuningLevel >= frogpilotToggleLevels[key].toDouble());
  }

  setUpdatesEnabled(true);
  update();
}

void FrogPilotModelPanel::hideSubToggles() {
  setUpdatesEnabled(false);

  if (modelRandomizerOpen) {
    for (auto &[key, toggle] : toggles) {
      toggle->setVisible(modelRandomizerKeys.find(key) != modelRandomizerKeys.end());
    }
  }

  setUpdatesEnabled(true);
  update();
}
