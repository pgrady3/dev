/** 
 * Fixed-point based FOC functions
 * 
 * This file contains the math functions
 * necessary to perform FOC calucations on 
 * a microprocessor lacking a floating-point
 * unit. All calculations are performed using
 * fixed-point math with integers.
 */

// Number of "decimal" places. Fixed point math will consider a value
// of 2^Q equal to 1.0
#define FIXED_Q		16	// 65536 = 1.0, smallest number = 0.00001526
#define ONE_HALF		((int32_t)( 32768U))
#define TWO				((int32_t)(131072U))
#define SQRT_3			((int32_t)(113512U))	
#define SQRT_3_OVER_2	((int32_t)( 56756U))
#define INV_SQRT_3		((int32_t)( 37837U))
#define MUL_LONG(a,b)	(int32_t)(((int64_t(a))*(int64_t(b))) >> FIXED_Q)
#define MUL_INT(a,b,)	(int32_t)(((int32_t(a))*(int32_t(b))) >> FIXED_Q)

typedef struct _clarkestruct
{
	int32_t As;
	int32_t Bs;
	int32_t Alpha;
	int32_t Beta;
} Clarke_Type;

typedef struct _parkstruct
{
	int32_t Alpha;
	int32_t Beta;
	int32_t Theta;
	int32_t Ds;
	int32_t Qs;
} Park_Type;

typedef struct _svmstruct
{
	int32_t Alpha;
	int32_t Beta;
	int32_t tA;
	int32_t tB; 
	int32_t tC;
} SVM_Type;

void clarke_transform(Clarke_Type* cs)
{
	// Alpha = A
	// Beta = (2*B + A) / sqrt(3)

	cs->Alpha = cs->As;
	temp = 2*(cs->Bs);
	cs->Beta = MUL_LONG(cs->As + MUL_LONG(cs->Bs,TWO), INV_SQRT_3); 
}

void park_transform(Park_Type* ps)
{
	// D = Alpha*cos(theta) + Beta*sin(theta)
	// Q = Beta*cos(theta) - Alpha*sin(theta)
	int32_t Sin, Cos;
	
	// TODO: Write sin, cos functions. Probably LUT based.
	Sin = foc_sin(ps->Theta);
	Cos = foc_cos(ps->Theta);
	ps->Ds = MUL_LONG(ps->Alpha, Cos) + MUL_LONG(ps->Beta, Sin);
	ps->Qs = MUL_LONG(ps->Beta, Cos) - MUL_LONG(ps->Alpha, Sin);
}

void inv_park_transform(Park_Type* ps)
{
	// Alpha = D*cos(theta) - Q*sin(theta)
	// Beta = Q*cos(theta) + D*sin(theta)
	int32_t Sin, Cos;
	
	// TODO: Still need sin and cos functions
	Sin = foc_sin(ps->theta);
	Cos = foc_cos(ps->theta);
	ps->Alpha = MUL_LONG(ps->Ds, Cos) - MUL_LONG(ps->Qs, Sin);
	ps->Beta = MUL_LONG(ps->Qs, Cos) + MUL_LONG(ps->Ds, Sin);
}

void svm_calc(SVM_Type* svm)
{
	
	// Sector determination
	uint8_t sector = 0;
	int32_t X,Y,Z,T1,T2;
	X = svm->Beta;
	Y = MUL_LONG(-ONE_HALF,svm->Beta) - MUL_LONG(SQRT_3_OVER_2,svm->Alpha);
	Z = MUL_LONG(-ONE_HALF,svm->Beta) + MUL_LONG(SQRT_3_OVER_2,svm->Alpha);

/** Determining the sector
 * 
 *       beta
 *        ^
 *        |
 *        |
 *  _____________> alpha
 *        |
 *        |
 *        |
 *
 *        X   
 *     \  |  /
 *      \ | /
 *       \|/
 *       /|\
 *      / | \
 *     /  |  \
 *    Y       Z
 *
 * If X is positive (beta is positive), sector is in the upper half plane
 * If Y is positive, sector is in the lower left side
 * If Z is positive, sector is in the lower right side
 *
 *
 *    \       / 
 *     \  1  /  
 *      \   /  
 *  __3_______5___
 *    2 /   \ 4
 *     /  6  \ 
 *    /       \	
 */
 
	if(X > 0)
		sector+=1;
	if(Y>0)
		sector+=2;
	if(Z>0)
		sector+=4;
/* 
 * The remaining assignments determine how much time
 * in the two nearest space vectors (T1 and T2) are
 * required to create the same vector as described by
 * the inputs, alpha and beta. Then, using the space
 * vector definitions (ie S1 = A+, B-, C-) the on-times
 * for phases A, B, and C are calculated.
 */

	switch(sector)
	{
	case 5:
		T1 = Z;
		T2 = X;
		svm->tC = 0;
		svm->tB = T2;
		svm->tA = svm->tB + T1;
		break;
	case 1:
		T1 = -Y;
		T2 = -Z;
		svm->tC = 0;
		svm->tA = T1;
		svm->tB = svm->tA + T2;
		break;
	case 3:
		T1 = X;
		T2 = Y;
		svm->tA = 0;
		svm->tC = T2;
		svm->tB = svm->C + T1;
		break;
	case 2:
		T1 = -Z;
		T2 = -X;
		svm->tA = 0;
		svm->tB = T1;
		svm->tC = svm->tB + T2;
		break;
	case 6:
		T1 = Y;
		T2 = Z;
		svm->tB = 0;
		svm->tA = T2;
		svm->tC = svm->tA + T1;
		break;
	case 4:
		T1 = -X;
		T2 = -Y;
		svm->tB = 0;
		svm->tC = T1;
		svm->tA = svm->tC + T2;
		break;
	default:
		svm->tA = 0;
		svm->tB = 0;
		svm->tC = 0;
		break;
	}
}