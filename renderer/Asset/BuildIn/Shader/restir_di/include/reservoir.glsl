// https://zhuanlan.zhihu.com/p/703950102

struct LightSample 
{
	// 被采样光源的信息
	uint lightID; 				// 光源索引
	
	float pHat;					// 当前光源的重要性采样权重，也就是targetPdf
	float sumWeights;			// 已处理的(pHat / proposalPdf)权重和
	float w;					// 被积函数在当前采样点对应的权重，同时也就是重采样重要性采样的realPdf(SIR PDF)的倒数
};

struct Reservoir 
{
	LightSample samples[RESERVOIR_SIZE];
	uint numStreamSamples;		// 已处理的采样总数，M 
};

uint ReservoirIndex(ivec2 pixel)
{
	return pixel.y * WINDOW_WIDTH + pixel.x;
}

void CleanReservoir(inout Reservoir res, int i)
{
	res.samples[i].lightID = 0;
	res.samples[i].pHat = 0.0f;
	res.samples[i].sumWeights = 0.0f;
	res.samples[i].w = 0.0f;
}

Reservoir NewReservoir() 
{
	Reservoir result;
	for (int i = 0; i < RESERVOIR_SIZE; ++i) 
	{
		CleanReservoir(result, i);
	}
	result.numStreamSamples = 0;
	return result;
}

void UpdateReservoir(
	inout Reservoir res, int i, float weight, 
	uint lightID,
	float pHat, 
	inout Rand rand) 
{
	res.samples[i].sumWeights += weight;											// 更新总计权重	
	if (RandFloat(rand) < (weight / (res.samples[i].sumWeights + 0.00001))) 		// 按Reservoir更新原则更新样本
	{										
		res.samples[i].lightID = lightID;
		res.samples[i].pHat = pHat;
	}
}

void AddSampleToReservoir(
	inout Reservoir res, 
	uint lightID, 
	float pHat, float proposalPdf, 
	inout Rand rand) 
{
	res.numStreamSamples = max(1, res.numStreamSamples + 1);						// 更新已处理采样数

	for (int i = 0; i < RESERVOIR_SIZE; ++i) 
	{
		if(proposalPdf <= 0) break;

		float weight = pHat / proposalPdf;												// 重要性重采样的权重，pdf/新分布下采样该点的概率
		UpdateReservoir(
			res, i, weight, 
			lightID, 
			pHat, 
			rand);

		if(res.samples[i].pHat <= 0) CleanReservoir(res, i);	
		else res.samples[i].w = (1 / res.samples[i].pHat) * ((res.samples[i].sumWeights) / (res.numStreamSamples));		// 被积函数对应的权重
	}	
}

// MIS For Proposals：
// 保持targetPdf不变，使用来自不同proposalPdf的样本，样本权重仍为各自的targetPdf(x) / proposalPdf_i(x), 
// 在每个proposalPdf分布同域时，合并无偏的

void CombineReservoirs(
	inout Reservoir self, 
	Reservoir other, 
	float pHat[RESERVOIR_SIZE], 
	inout Rand rand) 
{
	self.numStreamSamples += other.numStreamSamples;

	for (int i = 0; i < RESERVOIR_SIZE; i++) 
	{
		float weight = pHat[i] * other.samples[i].w * other.numStreamSamples;	// 将临近reservoir也视作一个（numStreamSamples个）样本：
		UpdateReservoir(														// proposalPdf是临近点的realPdf（SIR PDF）
			self, i, weight,													// targetPdf是待合并点的targetPdf
			other.samples[i].lightID, 
			pHat[i], 
			rand);

		if(self.samples[i].pHat <= 0) CleanReservoir(self, i);
		else self.samples[i].w = (1.0 / self.samples[i].pHat) * (1.0 / self.numStreamSamples) * self.samples[i].sumWeights;
	}
}

