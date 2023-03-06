#pragma once
/**
 * @brief 敌方装甲板颜色
 * 
 */
enum EnemyColor{
	RED = 0,
	BLUE = 1
};
/**
 * @brief 装甲板大小
 * 
 */
enum ArmorType{
	SMALL_ARMOR = 0,
	BIG_ARMOR = 1
};

/**
 * @brief 1-BIG_ARMOR, 3,4,5-SMALL_ARMOR
 */
enum ArmorNum{
	ONE = 1,
	THREE = 3,
	FOUR = 4,
	FIVE = 5
};

/**
 * @brief 识别追踪状态
 */
enum DetectorState
{
	LIGHTS_NOT_FOUND = 0,
	LIGHTS_FOUND = 1,
	ARMOR_NOT_FOUND = 2,
	ARMOR_FOUND = 3
};