// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
class TESTCPP_API MLP {

public:
	MLP();
	~MLP();

	static void initialize();
	static TArray<std::tuple<FString, int, int, TArray<float>>> weight_list;
	static FCriticalSection critical;

	TArray<float> run(TArray<float> input);
};
