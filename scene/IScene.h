#pragma once

class GameManager;

class IScene {
public:
	virtual ~IScene() {};
	virtual void Initialize(GameManager* gameManager) = 0;
	virtual void Update(GameManager* gameManager) = 0;
	virtual void Draw(GameManager* gameManager) = 0;
};