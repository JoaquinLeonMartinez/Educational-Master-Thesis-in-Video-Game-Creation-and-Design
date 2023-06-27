#pragma once

#include "modules/module.h"
#include "entity/entity.h"



class CModuleGameManager : public IModule
{
	enum MenuState;
	MenuState currentMenuState;
public:

	CModuleGameManager(const std::string& aname) : IModule(aname) { }
  bool start() override;
  void stop() override;
  void update(float delta) override;
  void renderDebug() override;
  void renderInMenu() override;
  bool menuVisible;
  bool victoryMenu;
  enum MenuState { menuNot, menuWin, menuDead_, menuPause };
  void setCurrentMenuState(MenuState pause);

private:
	void gameCondition();
	
	void setMenuState(MenuState pause);
	MenuState getCurrentMenuState();
	void exitPauseGame();
	bool menuDead;
	bool isPaused;

	 int windowWidth;
	 int windowHeight;
	 int menuPos = 0;
	 unsigned int menuSize = 1;
	ImGuiWindowFlags window_flags;

	
};

