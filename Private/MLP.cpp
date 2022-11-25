// Fill out your copyright notice in the Description page of Project Settings.


#include "MLP.h"
#include "AnimationRuntime.h"

MLP::MLP()
{
}

MLP::~MLP()
{
}

TArray<std::tuple<FString, int, int, TArray<float>>> MLP::weight_list;

FCriticalSection MLP::critical;


void MLP::initialize() {
	FScopeLock lock(&critical);
	weight_list.Reset();

	auto project_dir = FPaths::ProjectDir();
	auto binary_path = project_dir + TEXT("zlw_mlp_cp1.bin");
	UE_LOG(LogTemp, Warning, TEXT("current project_dir is %s"), *project_dir);
	UE_LOG(LogTemp, Warning, TEXT("current binary_path is %s"), *binary_path);


	auto binary_path_ansi = TCHAR_TO_ANSI(*binary_path);


	auto fp = fopen(binary_path_ansi, "rb");
	auto file = fseek(fp, 0L, SEEK_END);
	auto file_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	auto buffer = new BYTE[file_size];
	fread(buffer, 1, file_size, fp);
	fclose(fp);

	auto pos = buffer;

	FString logo;
	logo.Append((const char*)pos, 8);
	pos += 8;

	UE_LOG(LogTemp, Warning, TEXT("current buff is %s"), *logo);

	int length_name;
	int length_height;
	int length_width;

	int T_max = 10;
	size_t r = 0;
	while (T_max-- && pos - buffer < file_size) {
		length_name = *(int*)pos;
		pos += 4;

		FString weight_name;
		weight_name.Append((const char*)pos, length_name);
		pos += length_name;

		length_height = *(int*)pos;
		pos += 4;

		length_width = *(int*)pos;
		pos += 4;

		UE_LOG(LogTemp, Warning, TEXT("read block %s with shape %dx%d"), *weight_name, length_height, length_width);

		TArray<float> weight_data((float*)pos, length_height * length_width);
		pos += length_height * length_width * 4;

		weight_list.Add({ weight_name,length_height,length_width,weight_data });
		UE_LOG(LogTemp, Warning, TEXT("read float %f"), weight_data[length_height * length_width - 1]);
	}


}


TArray<float> multiply(int left_height, int left_width, TArray<float> left, TArray<float> right) {
	int right_height = left_height;
	int right_width = 1;
	TArray<float> result;
	result.Reset(right_height * right_width);
	result.AddZeroed(right_height * right_width);

	for (int left_row = 0; left_row < left_height; left_row++) {
		for (int left_col = 0; left_col < left_width; left_col++) {
			int right_row = left_row;
			int right_col = 0;
			result[right_row * right_width + right_col] += left[left_row * left_width + left_col] * right[left_col];
		}
	}

	return result;
}


TArray<float> MLP::run(TArray<float> input)
{
	for (int i = 0; i < weight_list.Num(); i++) {
		FString weight_name;
		int length_height;
		int length_width;
		TArray<float> weight_data;
		std::tie(weight_name, length_height, length_width, weight_data) = weight_list[i];
		auto result = multiply(length_height, length_width, weight_data, input);

		UE_LOG(LogTemp, Warning, TEXT("run %s and get shape %dx1"), *weight_name, result.Num());

		return result;
	}

	return TArray<float>();
}


