#include "stdafx.h"
#include "../Core/Resource.h"
#include <mmsystem.h>
#include <ddraw.h>
#include "../Library/audio.h"
#include "../Library/gameutil.h"
#include "../Library/gamecore.h"
#include "mygame.h"


using namespace game_framework;

/////////////////////////////////////////////////////////////////////////////
// 這個class為遊戲的遊戲執行物件，主要的遊戲程式都在這裡
/////////////////////////////////////////////////////////////////////////////

CGameStateRun::CGameStateRun(CGame* g) : CGameState(g)
{
}

CGameStateRun::~CGameStateRun()
{
}

void CGameStateRun::OnBeginState()
{
}

void CGameStateRun::OnMove()							// 移動遊戲元素
{
	//player.setPower(15);
	if (mainStage == GAME_STAGE) {
		if (isPause||isDead)
			return;
		
		player.reduceInvincible();
		// player
		for (size_t i = 0; i < enemyBullets.size(); i++)
		{
			if (player.isDeath(enemyBullets[i]))
			{
				//被射中的時候
				player.setInvincible(120);
				player.setPower(0);
				player.setRemainingLives(player.getRemainingLives() - 1);
				player.setRangeAnimation(0, 3, 100, false);//not enough
				isInvincibleCount = true;

			}
		}
		player.updateLocationFBySpeed();
		fixPlayerLocation();
		//換回正常模式動畫
		if (isInvincibleCount == true) {
			if (player.getInvincible() == 0) {
				player.setRangeAnimation(1, 3, 100, false);
				isInvincibleCount = false;
			}
		}
		if (player.getRemainingLives() == 0)
			isDead=true;
		


		// generate player bullet, move bullet and erase
		updatePlayerBullet();
		checkBulletHitEnemy();

		// falling object
		for (size_t i = 0; i < fallingObjects.size(); i++)
		{
			fallingObjects[i].updateLocationFBySpeed();
			if (fallingObjects[i].IsOverlap(fallingObjects[i], player))
			{
				fallingObjects.erase(fallingObjects.begin() + i);
				i--;
				player.setPower(player.getPower() + 1);
			}
		}
		// generate enemy, move enemy and erase enemy leave player area
		// same of enemy bullet
		updateEnemy();
		
		// boss
		std::shared_ptr<Boss> bossCreated = MapCreator::getCurrentBoss(frameCounter, playerArea);
		if (bossCreated != nullptr)
		{
			this->boss = bossCreated;
		}
		if (this->boss != nullptr)
		{
			this->boss->update(&player, &enemyBullets, &playerArea);
			if (this->boss->onLeave(playerArea))
			{
				this->boss = nullptr;
			}
		}
		frameCounter += 1;
	}
}

void CGameStateRun::OnInit()  								// 遊戲的初值及圖形設定
{
	initMenu();
	initGame();
	GotoGameState(GAME_STATE_RUN);
}

void CGameStateRun::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (mainStage == MENU_STAGE) {
		if (nChar == VK_UP) {
			if (menuStage == MAIN_MENU)
				setMainMenuSelection(-1);
;
		}
		else if (nChar == VK_DOWN) {
			if (menuStage == MAIN_MENU)
				setMainMenuSelection(1);

		}
		// key z
		else if (nChar == 0x5A) {
			if (menuStage == MAIN_MENU)
				if (mainMenuButtonSelectIndex == START)
					mainStage = GAME_STAGE;
		}
	}
	else {
		if (nChar == VK_UP) {
			player.setSpeedY(player.getSpeedY() - playerDelta);
			if (isPause)
				setPauseButtionSelection(-1);
			if (isDead)
				setDeadButtionSelection(-1);
		}
		else if (nChar == VK_DOWN) {
			player.setSpeedY(player.getSpeedY() + playerDelta);
			if (isPause)
				setPauseButtionSelection(1);
			if (isDead)
				setDeadButtionSelection(1);
		}
		else if (nChar == VK_LEFT) {
			player.setSpeedX(player.getSpeedX() - playerDelta);
		}
		else if (nChar == VK_RIGHT) {
			player.setSpeedX(player.getSpeedX() + playerDelta);

		}
		else if (nChar == 0x5A) {
			fire = true;
			if (isPause) {
				if (pauseButtionIndex == 0)
					isPause = false;
				if (pauseButtionIndex == 1) {
					mainStage = MENU_STAGE;
					resetGame();
				}				
			}
			if (isDead)
			{
				if (deadButtionIndex == 0) {
					//continue
				}
				if (deadButtionIndex == 1) {
					mainStage = MENU_STAGE;
					resetGame();
				}

			}
		}
		else if (nChar == VK_ESCAPE) {
			if (isPause)
				isPause = false;
			else
				isPause = true;
		}
	}

}
void CGameStateRun::resetGame() {
	isPause = false;
	isDead = false;
	player = ReimuB();////////////
	playerBullets.clear();
	enemies.clear();
	enemyBullets.clear();
	fallingObjects.clear();
	frameCounter = 0;
	player.onInit();
}

void CGameStateRun::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (mainStage == GAME_STAGE) {
		if (nChar == VK_UP) {
			player.setSpeedY(player.getSpeedY() + playerDelta);
		}
		else if (nChar == VK_DOWN) {
			player.setSpeedY(player.getSpeedY() - playerDelta);
		}
		else if (nChar == VK_LEFT) {
			player.setSpeedX(player.getSpeedX() + playerDelta);
			//player.setRangeAnimation(1, 3, 100, true);
			//player.startToggleAnimation();
		}
		else if (nChar == VK_RIGHT) {
			player.setSpeedX(player.getSpeedX() - playerDelta);
			/*
			*/
		}
		else if (nChar == 0x5A) {
			fire = false;
		}
	}
}

void CGameStateRun::OnLButtonDown(UINT nFlags, CPoint point)  // 處理滑鼠的動作
{
}

void CGameStateRun::OnLButtonUp(UINT nFlags, CPoint point)	// 處理滑鼠的動作
{
}

void CGameStateRun::OnMouseMove(UINT nFlags, CPoint point)	// 處理滑鼠的動作
{
}

void CGameStateRun::OnRButtonDown(UINT nFlags, CPoint point)  // 處理滑鼠的動作
{
}

void CGameStateRun::OnRButtonUp(UINT nFlags, CPoint point)	// 處理滑鼠的動作
{
}


void CGameStateRun::OnShow()
{
	if (mainStage == MENU_STAGE) {
		background.ShowBitmap();
		showMainMenuButtons();
	}
	else {
		if (isPause) {
			pauseTitle.ShowBitmap();
			showPauseButtion();
		}
		if (isDead) {
			deadTitle_01.ShowBitmap();
			deadTitle_02.ShowBitmap();
			deadLivesShower.showNumber(player.getRemainingLives());
			showDeadButtion();
		}
			
		showGame();
	}
}

void CGameStateRun::showMainMenuButtons() {
	int d = 3;
	for (size_t i = 0; i < mainMenuButtons.size(); i++)
	{
		CMovingBitmap button = mainMenuButtons[i];
		if (button.GetFrameIndexOfBitmap() == 0 && mainMenuButtonSelectIndex == i) {
			button.SetTopLeft(button.GetLeft() - d, button.GetTop() - d);
			button.SetFrameIndexOfBitmap(1);
		}
		else if (button.GetFrameIndexOfBitmap() == 1 && mainMenuButtonSelectIndex != i) {
			button.SetTopLeft(button.GetLeft() + d, button.GetTop() + d);
			button.SetFrameIndexOfBitmap(0);
		}
		button.ShowBitmap();
	}
}

void CGameStateRun::showPauseButtion() {
	int d = 3;
	for (size_t i = 0; i < pauseButtons.size(); i++)
	{
		CMovingBitmap button = pauseButtons[i];
		if (button.GetFrameIndexOfBitmap() == 0 && pauseButtionIndex == i) {
			button.SetTopLeft(button.GetLeft() - d, button.GetTop() - d);
			button.SetFrameIndexOfBitmap(1);
		}
		else if (button.GetFrameIndexOfBitmap() == 1 && pauseButtionIndex != i) {
			button.SetTopLeft(button.GetLeft() + d, button.GetTop() + d);
			button.SetFrameIndexOfBitmap(0);
		}
		button.ShowBitmap();
	}
}
void CGameStateRun::showDeadButtion() {
	int d = 3;
	for (size_t i = 0; i < deadButtons.size(); i++)
	{
		CMovingBitmap button = deadButtons[i];
		if (button.GetFrameIndexOfBitmap() == 0 && deadButtionIndex == i) {
			button.SetTopLeft(button.GetLeft() - d, button.GetTop() - d);
			button.SetFrameIndexOfBitmap(1);
		}
		else if (button.GetFrameIndexOfBitmap() == 1 && deadButtionIndex != i) {
			button.SetTopLeft(button.GetLeft() + d, button.GetTop() + d);
			button.SetFrameIndexOfBitmap(0);
		}
		button.ShowBitmap();
	}
}

void CGameStateRun::initMenu() {
	// menu background
	background.LoadBitmapByString({ "Resources\\Image\\TL\\title00\\Sprite0.bmp" });
	background.SetTopLeft(0, 0);
	// main menu
	vector<vector<string>> buttonImagePaths = {
		{ "Resources\\Image\\TL\\title01\\Sprite10.bmp","Resources\\Image\\TL\\title01s\\Sprite10.bmp" },
		{ "Resources\\Image\\TL\\title01\\Sprite11.bmp","Resources\\Image\\TL\\title01s\\Sprite11.bmp" },
		{ "Resources\\Image\\TL\\title01\\Sprite26.bmp","Resources\\Image\\TL\\title01s\\Sprite26.bmp" },
		{ "Resources\\Image\\TL\\title01\\Sprite12.bmp","Resources\\Image\\TL\\title01s\\Sprite12.bmp" },
		{ "Resources\\Image\\TL\\title01\\Sprite13.bmp","Resources\\Image\\TL\\title01s\\Sprite13.bmp" },
		{ "Resources\\Image\\TL\\title01\\Sprite25.bmp","Resources\\Image\\TL\\title01s\\Sprite25.bmp" },
		{ "Resources\\Image\\TL\\title01\\Sprite15.bmp","Resources\\Image\\TL\\title01s\\Sprite15.bmp" },
		{ "Resources\\Image\\TL\\title01\\Sprite14.bmp","Resources\\Image\\TL\\title01s\\Sprite14.bmp" },
	};
	vector<vector<int>> buttonLocations = {
		{447,199},
		{438, 227},
		{428, 256},
		{418, 284},
		{408, 312},
		{398, 340},
		{388, 369},
		{378, 396}
	};
	for (size_t i = 0; i < buttonImagePaths.size(); i++)
	{
		CMovingBitmap button;
		button.LoadBitmapByString(buttonImagePaths[i], RGB(0, 0, 0));
		button.SetTopLeft(buttonLocations[i][0], buttonLocations[i][1]);
		mainMenuButtons.push_back(button);
	}
	//stop manu 
	//"Resources\\Image\\IN\\ascii\\Sprite111.bmp" //遊戲暫停

	vector<vector<string>> pauseButtonPaths = {
		{ "Resources\\Image\\IN\\ascii\\Sprite112.bmp","Resources\\Image\\IN\\asciis\\Sprite111.bmp" },//解除暫停
		{ "Resources\\Image\\IN\\ascii\\Sprite113.bmp","Resources\\Image\\IN\\asciis\\Sprite112.bmp" },//返回標題
	};
	vector<vector<int>> pauseButtonLocations = {
		{180,230},
		{180,250},
	};
	for (size_t i = 0; i < pauseButtonPaths.size(); i++)
	{
		CMovingBitmap button;
		button.LoadBitmapByString(pauseButtonPaths[i], RGB(0, 0, 0));
		button.SetTopLeft(pauseButtonLocations[i][0], pauseButtonLocations[i][1]);
		pauseButtons.push_back(button);
	}

	vector<vector<string>> deadButtonPaths = {
		{ "Resources\\Image\\IN\\ascii\\Sprite115.bmp","Resources\\Image\\IN\\asciis\\Sprite114.bmp" },//是
		{ "Resources\\Image\\IN\\ascii\\Sprite116.bmp","Resources\\Image\\IN\\asciis\\Sprite115.bmp" },//否
	};

	vector<vector<int>> deadButtonLocations = {
	{200,230},
	{200,250},
	};

	for (size_t i = 0; i < deadButtonPaths.size(); i++)
	{
		CMovingBitmap button;
		button.LoadBitmapByString(deadButtonPaths[i], RGB(0, 0, 0));
		button.SetTopLeft(deadButtonLocations[i][0], deadButtonLocations[i][1]);
		deadButtons.push_back(button);
	}
}


void CGameStateRun::setMainMenuSelection(int direction) {
	mainMenuButtonSelectIndex += direction;
	if (mainMenuButtonSelectIndex < 0)
		mainMenuButtonSelectIndex = 0;
	else if (mainMenuButtonSelectIndex == mainMenuButtons.size())
		mainMenuButtonSelectIndex = mainMenuButtons.size() - 1;
}
void CGameStateRun::setPauseButtionSelection(int direction) {
	pauseButtionIndex += direction;
	if (pauseButtionIndex < 0)
		pauseButtionIndex = 0;
	else if (pauseButtionIndex == pauseButtons.size())
		pauseButtionIndex = pauseButtons.size() - 1;
}
void CGameStateRun::setDeadButtionSelection(int direction) {
	deadButtionIndex += direction;
	if (deadButtionIndex < 0)
		deadButtionIndex = 0;
	else if (deadButtionIndex == deadButtons.size())
		deadButtionIndex = deadButtons.size() - 1;
}


void CGameStateRun::initGame() {
	// player
	//player.LoadBitmapByString({ "Resources\\Image\\CM\\player00\\Sprite0.bmp" }, RGB(205, 205, 205));
	pauseTitle.LoadBitmapByString({ "Resources\\Image\\IN\\ascii\\Sprite111.bmp" }, RGB(0, 0, 0));
	pauseTitle.SetTopLeft(195, 186);




	deadTitle_01.LoadBitmapByString({ "Resources\\Image\\IN\\ascii\\Sprite117.bmp" }, RGB(0, 0, 0));
	deadTitle_01.SetTopLeft(150,186);

	deadTitle_02.LoadBitmapByString({ "Resources\\Image\\IN\\ascii\\Sprite118.bmp" }, RGB(0, 0, 0));
	deadTitle_02.SetTopLeft(220, 206);


	player.onInit();
	
	// player moveing area
	playerArea.LoadEmptyBitmap(448, 384);
	playerArea.setLocationF(32.0f, 16.0f);

	//player
	//player.startToggleAnimation();
	// interface background
	for (const int filename : {5, 6, 7, 23}) {
		CMovingBitmap interfaceBackgound;
		interfaceBackgound.LoadBitmapByString({ "Resources\\Image\\CM\\front\\Sprite" + to_string(filename) + ".bmp" });
		interfaceBackgroundResource.push_back(interfaceBackgound);
	}
	// live and bomb star
	RedStar.LoadBitmapByString({ "Resources\\Image\\CM\\front\\Sprite17.bmp" }, RGB(0, 0, 0));
	GreenStar.LoadBitmapByString({ "Resources\\Image\\CM\\front\\Sprite18.bmp" }, RGB(0, 0, 0));
	// interface labels
	vector<vector<string>> labelImagePaths = {
		{ "Resources\\Image\\CM\\front\\Sprite9.bmp" },
		{ "Resources\\Image\\CM\\front\\Sprite8.bmp" },
		{ "Resources\\Image\\CM\\front\\Sprite10.bmp" },
		{ "Resources\\Image\\CM\\front\\Sprite11.bmp" },
		{ "Resources\\Image\\CM\\front\\Sprite15.bmp" },
		{ "Resources\\Image\\CM\\front\\Sprite16.bmp" },
		{ "Resources\\Image\\CM\\front\\Sprite20.bmp" },
	};
	vector<vector<int>> labelLocations = {
		{432, 58},
		{431, 82},
		{432, 121},
		{432, 146},
		{432, 186},
		{431, 206},
		{432, 226}
	};
	for (size_t i = 0; i < labelImagePaths.size(); i++)
	{
		CMovingBitmap label;
		label.LoadBitmapByString(labelImagePaths[i], RGB(0, 0, 0));
		label.SetTopLeft(labelLocations[i][0], labelLocations[i][1]);
		gameInterface.push_back(label);
	}
	// number system
	// show score and player data
	for (int i = 0; i < 5; i++) {
		NumberDisplay numberDisplay;
		numberDisplay.onInit();
		numberDisplays.push_back(numberDisplay);
	}
	numberDisplays[0].setMinDigit(9);
	numberDisplays[0].setXY(496, 58);
	numberDisplays[1].setMinDigit(9);
	numberDisplays[1].setXY(496, 82);
	numberDisplays[2].setXY(496, 186);
	numberDisplays[3].setXY(496, 206);
	numberDisplays[4].setXY(496, 226);

	//dead reLives shower
	deadLivesShower.onInit();
	deadLivesShower.setXY(252,206);

	MapCreator::onInit(&playerArea, &mapDatum);
}

void CGameStateRun::showGame() {
	// player
	//player.SetAnimation(1, false);
	player.handMadeShow();

	//player.ShowBitmap();
	// player bullets
	for (size_t i = 0; i < playerBullets.size(); i++)
		playerBullets[i].ShowBitmap();
	// falling object
	for (size_t i = 0; i < fallingObjects.size(); i++)
		fallingObjects[i].ShowBitmap();
	// enemy
	for (size_t i = 0; i < enemies.size(); i++)
		enemies[i].ShowBitmap();
	// enemy bullets
	for (size_t i = 0; i < enemyBullets.size(); i++)
		enemyBullets[i].ShowBitmap();
	// boss
	if (this->boss != nullptr) {
		boss->show();
		boss->showDisplay();
	}

	// interface
	// interface border
	showBorder();
	// interfaece labe
	for (size_t i = 0; i < gameInterface.size(); i++)
		gameInterface[i].ShowBitmap();
	// number system
	numberDisplays[0].showNumber(1000000);
	numberDisplays[1].showNumber(0);
	numberDisplays[2].showNumber(player.getPower());
	numberDisplays[3].showNumber(0);
	numberDisplays[4].showNumber(0);
	// player star
	for (int i = 0; i < player.getRemainingLives(); i++)
	{
		RedStar.SetTopLeft(496 + i * RedStar.GetWidth(), 122);
		RedStar.ShowBitmap();
	}
	// bomb star
	for (int i = 0; i < Bomb; i++)
	{
		GreenStar.SetTopLeft(496 + i * GreenStar.GetWidth(), 146);
		GreenStar.ShowBitmap();
	}
	if (player.getPower() >= 8) {
		player.showBall();
	}
}

void CGameStateRun::showBorder() {
	// right border
	CMovingBitmap leftBackground = interfaceBackgroundResource[0];
	for (int y = 0; y < 480; y += leftBackground.GetHeight())
	{
		leftBackground.SetTopLeft(0, y);
		leftBackground.ShowBitmap();
	}
	// top, bottom border
	CMovingBitmap topBackground = interfaceBackgroundResource[1];
	CMovingBitmap bottomBackground = interfaceBackgroundResource[2];
	for (int x = topBackground.GetWidth(); x < playerArea.GetLeft() + playerArea.GetWidth(); x += topBackground.GetWidth())
	{
		topBackground.SetTopLeft(x, 0);
		topBackground.ShowBitmap();
		bottomBackground.SetTopLeft(x, 480 - bottomBackground.GetHeight());
		bottomBackground.ShowBitmap();
	}
	// right border
	CMovingBitmap rightBackground = interfaceBackgroundResource[3];
	for (int y = 0; y < 480; y += rightBackground.GetHeight())
	{
		for (int x = playerArea.GetLeft() + playerArea.GetWidth(); x < 640; x += rightBackground.GetWidth())
		{
			rightBackground.SetTopLeft(x, y);
			rightBackground.ShowBitmap();
		}
	}
}

void CGameStateRun::fixPlayerLocation() {
	// hit top bound
	if (player.getLocationF().y < playerArea.GetTop()) {
		player.setLocationF(player.getLocationF().x, (float)playerArea.GetTop());
	}
	// hit left bound
	if (player.getLocationF().x < playerArea.GetLeft())
	{
		player.setLocationF((float)playerArea.GetLeft(), player.getLocationF().y);
	}
	// hit bottom bound
	if (player.getLocationF().y + player.GetHeight() > playerArea.GetTop() + playerArea.GetHeight()) {
		player.setLocationF(player.getLocationF().x, (float)(playerArea.GetTop() + playerArea.GetHeight() - player.GetHeight()));
	}
	// hit right bound
	if (player.getLocationF().x + player.GetWidth() > playerArea.GetLeft() + playerArea.GetWidth())
	{
		player.setLocationF((float)(playerArea.GetLeft() + playerArea.GetWidth() - player.GetWidth()), player.getLocationF().y);
	}
}

void CGameStateRun::checkBulletHitEnemy() {
	for (size_t i = 0; i < playerBullets.size(); i++) {
		for (size_t j = 0; j < enemies.size(); j++) {
			if (enemies[j].getHitable() && playerBullets[i].IsOverlap(playerBullets[i], enemies[j])) {
				addFallingObject(enemies[j]);
				playerBullets.erase(playerBullets.begin() + i);
				enemies.erase(enemies.begin() + j);
				break;
			}
		}
	}
}

void CGameStateRun::updateEnemy()
{
	// generate emeny
	map<size_t, vector<MapData>>::iterator iter = mapDatum.find(frameCounter);
	if (iter != mapDatum.end()) {
		for (size_t i = 0; i < iter->second.size(); i++)
		{
			Enemy enemy;
			enemy.LoadBitmapByString(iter->second[i].resource, iter->second[i].colorFilter);
			enemy.setLocationF(iter->second[i].location.x, iter->second[i].location.y);
			enemy.setHitable(iter->second[i].hitable);
			if (iter->second[i].aimTarget == MapData::AIM_TARGET::NO) {
				enemy.setSpeeds(iter->second[i].speeds);
			}
			else {
				double angle2Player = enemy.angle2Target(player);
				float speed = 3;
				float x = (float)cos(angle2Player * M_PI / 180) * speed;
				float y = (float)sin(angle2Player * M_PI / 180) * speed;
				enemy.setSpeed(POINTF{ x,y });
			}

			enemy.setAction(iter->second[i].enemyAction);
			enemies.push_back(enemy);
		}
	}

	// erase enemy and move enemy
	for (size_t i = 0; i < enemies.size(); i++) {
		if (enemies[i].onLeave(playerArea)) {
			enemies.erase(enemies.begin() + i);
			i--;
			continue;
		}
		else {
			enemies[i].update(&player, &enemyBullets);
		}
	}

	// enemy bullets
	for (size_t i = 0; i < enemyBullets.size(); i++) {
		if (!enemyBullets[i].IsOverlap(enemyBullets[i], playerArea))
		{
			enemyBullets.erase(enemyBullets.begin() + i);
			i--;
			continue;
		}
		else {
			enemyBullets[i].updateLocationFBySpeed();
		}
	}
}

void CGameStateRun::updatePlayerBullet()
{
	// erase bullet and move bullet
	for (size_t i = 0; i < playerBullets.size(); i++)
	{
		if (!playerBullets[i].IsOverlap(playerBullets[i], playerArea))
		{
			playerBullets.erase(playerBullets.begin() + i);
			i--;
			continue;
		}
		else {
			playerBullets[i].updateLocationFBySpeed();
		}
	}

	// generate
	if (fire) {
		player.setPower(70);
		vector<MovingObject> ms = player.attack();
		playerBullets.insert(playerBullets.end(), ms.begin(), ms.end());//wave1.insert(wave1.end(), curve1Speeds.begin() + 1, curve1Speeds.end());
	}
}

void CGameStateRun::addFallingObject(MovingObject enemy) {
	MovingObject falling;
	falling.LoadBitmapByString({ "Resources\\Image\\CM\\etama3\\Sprite0.bmp" }, RGB(65, 52, 52));
	float left = enemy.getLocationF().x + (enemy.GetWidth() - falling.GetWidth()) / 2.0f;
	float top = enemy.getLocationF().y + (enemy.GetHeight() - falling.GetHeight()) / 2.0f;
	falling.setLocationF(left, top);
	falling.setSpeed(POINTF{ 0, 3 });
	fallingObjects.push_back(falling);
}
