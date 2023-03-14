#include "math.h"
#include "../Interface/interfaces.h"
#include <random>

bool M::Setup()
{
	RandomSeed = reinterpret_cast<RandomSeedFn>(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomSeed"));
	if (RandomSeed == nullptr)
		return false;

	RandomFloat = reinterpret_cast<RandomFloatFn>(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat"));
	if (RandomFloat == nullptr)
		return false;

	RandomFloatExp = reinterpret_cast<RandomFloatExpFn>(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloatExp"));
	if (RandomFloatExp == nullptr)
		return false;

	RandomInt = reinterpret_cast<RandomIntFn>(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomInt"));
	if (RandomInt == nullptr)
		return false;

	RandomGaussianFloat = reinterpret_cast<RandomGaussianFloatFn>(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomGaussianFloat"));
	if (RandomGaussianFloat == nullptr)
		return false;

	return true;
}

void M::VectorAngles(const Vector& vecForward, Vector& angView)
{
	float flPitch, flYaw;

	if (vecForward.x == 0.f && vecForward.y == 0.f)
	{
		flPitch = (vecForward.z > 0.f) ? 270.f : 90.f;
		flYaw = 0.f;
	}
	else
	{
		flPitch = std::atan2f(-vecForward.z, vecForward.Length2D()) * 180.f / M_PI;

		if (flPitch < 0.f)
			flPitch += 360.f;

		flYaw = std::atan2f(vecForward.y, vecForward.x) * 180.f / M_PI;

		if (flYaw < 0.f)
			flYaw += 360.f;
	}

	angView.x = flPitch;
	angView.y = flYaw;
	angView.z = 0.f;
}

void M::AngleVectors(const Vector& angView, Vector* pForward, Vector* pRight, Vector* pUp)
{
	float sp, sy, sr, cp, cy, cr;

	DirectX::XMScalarSinCos(&sp, &cp, M_DEG2RAD(angView.x));
	DirectX::XMScalarSinCos(&sy, &cy, M_DEG2RAD(angView.y));
	DirectX::XMScalarSinCos(&sr, &cr, M_DEG2RAD(angView.z));

	if (pForward != nullptr)
	{
		pForward->x = cp * cy;
		pForward->y = cp * sy;
		pForward->z = -sp;
	}

	if (pRight != nullptr)
	{
		pRight->x = -1 * sr * sp * cy + -1 * cr * -sy;
		pRight->y = -1 * sr * sp * sy + -1 * cr * cy;
		pRight->z = -1 * sr * cp;
	}

	if (pUp != nullptr)
	{
		pUp->x = cr * sp * cy + -sr * -sy;
		pUp->y = cr * sp * sy + -sr * cy;
		pUp->z = cr * cp;
	}
}

void AngleMatrixCalculation( const Vector& angles, matrix3x4_t& matrix )
{
	float sr, sp, sy, cr, cp, cy;

	DirectX::XMScalarSinCos( &sp, &cp, M_DEG2RAD( angles.x ) );
	DirectX::XMScalarSinCos( &sy, &cy, M_DEG2RAD( angles.y ) );
	DirectX::XMScalarSinCos( &sr, &cr, M_DEG2RAD( angles.z ) );

	// matrix = (YAW * PITCH) * ROLL
	matrix[ 0 ][ 0 ] = cp * cy;
	matrix[ 1 ][ 0 ] = cp * sy;
	matrix[ 2 ][ 0 ] = -sp;

	float crcy = cr * cy;
	float crsy = cr * sy;
	float srcy = sr * cy;
	float srsy = sr * sy;
	matrix[ 0 ][ 1 ] = sp * srcy - crsy;
	matrix[ 1 ][ 1 ] = sp * srsy + crcy;
	matrix[ 2 ][ 1 ] = sr * cp;

	matrix[ 0 ][ 2 ] = ( sp * crcy + srsy );
	matrix[ 1 ][ 2 ] = ( sp * crsy - srcy );
	matrix[ 2 ][ 2 ] = cr * cp;

	matrix[ 0 ][ 3 ] = 0.0f;
	matrix[ 1 ][ 3 ] = 0.0f;
	matrix[ 2 ][ 3 ] = 0.0f;
}

void M::AngleMatrix( const Vector& ang, const Vector& pos, matrix3x4_t& out ) {
	AngleMatrixCalculation( ang, out );
	out.SetOrigin( pos );
}

void M::AngleMatrix(const Vector& angView, matrix3x4_t& matOutput, const Vector& vecOrigin)
{
	float sp, sy, sr, cp, cy, cr;

	DirectX::XMScalarSinCos(&sp, &cp, M_DEG2RAD(angView.x));
	DirectX::XMScalarSinCos(&sy, &cy, M_DEG2RAD(angView.y));
	DirectX::XMScalarSinCos(&sr, &cr, M_DEG2RAD(angView.z));

	matOutput.SetForward(Vector(cp * cy, cp * sy, -sp));

	const float crcy = cr * cy;
	const float crsy = cr * sy;
	const float srcy = sr * cy;
	const float srsy = sr * sy;

	matOutput.SetLeft(Vector(sp * srcy - crsy, sp * srsy + crcy, sr * cp));
	matOutput.SetUp(Vector(sp * crcy + srsy, sp * crsy - srcy, cr * cp));
	matOutput.SetOrigin(vecOrigin);
}

Vector2D M::AnglePixels(const float flSensitivity, const float flPitch, const float flYaw, const Vector& angBegin, const Vector& angEnd)
{
	Vector angDelta = angBegin - angEnd;
	angDelta.Normalize();

	const float flPixelMovePitch = (-angDelta.x) / (flYaw * flSensitivity);
	const float flPixelMoveYaw = (angDelta.y) / (flPitch * flSensitivity);

	return Vector2D(flPixelMoveYaw, flPixelMovePitch);
}

Vector M::PixelsAngle(const float flSensitivity, const float flPitch, const float flYaw, const Vector2D& vecPixels)
{
	const float flAngleMovePitch = (-vecPixels.x) * (flYaw * flSensitivity);
	const float flAngleMoveYaw = (vecPixels.y) * (flPitch * flSensitivity);

	return Vector(flAngleMoveYaw, flAngleMovePitch, 0.f);
}

Vector M::CalcAngle(const Vector& vecStart, const Vector& vecEnd)
{
	Vector angView;
	const Vector vecDelta = vecEnd - vecStart;
	VectorAngles(vecDelta, angView);
	angView.Normalize();

	return angView;
}

Vector M::VectorTransform(const Vector& vecTransform, const matrix3x4_t& matrix)
{
	return Vector(vecTransform.DotProduct(matrix[0]) + matrix[0][3],
		vecTransform.DotProduct(matrix[1]) + matrix[1][3],
		vecTransform.DotProduct(matrix[2]) + matrix[2][3]);
}

Vector M::ExtrapolateTick(const Vector& p0, const Vector& v0)
{
	// position formula: p0 + v0t
	return p0 + (v0 * i::GlobalVars->flIntervalPerTick);
}
	
Vector M::Interpolate(const Vector from, const Vector to, const float percent) {
	return to * percent + from * (1.f - percent);
}

float M::ApproachAngle(float flTarget, float flValue, float flSpeed) {

	float delta = flTarget - flValue;

	// Speed is assumed to be positive
	if (flSpeed < 0)
		flSpeed = -flSpeed;

	if (delta < -180)
		delta += 360;
	else if (delta > 180)
		delta -= 360;

	if (delta > flSpeed)
		flValue += flSpeed;
	else if (delta < -flSpeed)
		flValue -= flSpeed;
	else
		flValue = flTarget;

	return flValue;
}

// not the same as math::Lerp
float M::Interpolate(const float from, const float to, const float percent) {
	return to * percent + from * (1.f - percent);
}

void M::Extrapolate(CBaseEntity* pEnt , Vector& vecOrigin, Vector& vecVelocity, int& fFlags, bool bOnGround) {

	Vector                start, end, normal;
	CGameTrace            trace;
	CTraceFilter		  filter(nullptr, TRACE_WORLD_ONLY);

	// define trace start.
	start = vecOrigin;

	// move trace end one tick into the future using predicted velocity.
	end = start + (vecVelocity * i::GlobalVars->flIntervalPerTick);

	// trace.
	i::EngineTrace->TraceRay(Ray_t(start, end, pEnt->GetCollideable()->OBBMins(), pEnt->GetCollideable()->OBBMaxs()), CONTENTS_SOLID, &filter, &trace);

	// we hit shit
	// we need to fix shit.
	if (trace.flFraction != 1.f) {

		// fix sliding on planes.
		for (int i{ }; i < 2; ++i) {
			vecVelocity -= trace.plane.vecNormal * vecVelocity.DotProduct(trace.plane.vecNormal);

			float adjust = vecVelocity.DotProduct(trace.plane.vecNormal);
			if (adjust < 0.f)
				vecVelocity -= (trace.plane.vecNormal * adjust);

			start = trace.vecEnd;
			end = start + (vecVelocity * (i::GlobalVars->flIntervalPerTick * (1.f - trace.flFraction)));

			i::EngineTrace->TraceRay(Ray_t(start, end, pEnt->GetCollideable()->OBBMins(), pEnt->GetCollideable()->OBBMaxs()), CONTENTS_SOLID, &filter, &trace);
			if (trace.flFraction == 1.f)
				break;
		}
	}

	// set new final origin.
	start = end = vecOrigin = trace.vecEnd;

	// move endpos 2 units down.
	// this way we can check if we are in/on the ground.
	end.z -= 2.f;

	// trace.
	i::EngineTrace->TraceRay(Ray_t(start, end, pEnt->GetCollideable()->OBBMins(), pEnt->GetCollideable()->OBBMaxs()), CONTENTS_SOLID, &filter, &trace);

	// strip onground flag.
	fFlags &= ~FL_ONGROUND;

	// add back onground flag if we are onground.
	if (trace.flFraction != 1.f && trace.plane.vecNormal.z > 0.7f)
		fFlags |= FL_ONGROUND;
}

void M::RotatePoint(const ImVec2& vecIn, const float flAngle, ImVec2* pOutPoint)
{
	if (&vecIn == pOutPoint)
	{
		const ImVec2 vecPoint = vecIn;
		RotatePoint(vecPoint, flAngle, pOutPoint);
		return;
	}

	const float flSin = std::sinf(M_DEG2RAD(flAngle));
	const float flCos = std::cosf(M_DEG2RAD(flAngle));

	pOutPoint->x = vecIn.x * flCos - vecIn.y * flSin;
	pOutPoint->y = vecIn.x * flSin + vecIn.y * flCos;
}

void M::SinCos(float flRadian, float* flSin, float* flCos) {
	*flSin = std::sin(flRadian);
	*flCos = std::cos(flRadian);
}

float M::GenerateRandom(float flMin, float flMax) {

	static std::random_device rand_device;
	static std::mt19937 rand_gen(rand_device());

	std::uniform_real_distribution<float> rand_distributer(flMin, flMax);
	return rand_distributer(rand_gen);
}

float M::NormalizeYaw(float flYaw) {

	float yaww = flYaw;
	while (yaww < -180.0f) yaww += 360.0f;
	while (yaww > 180.0f) yaww -= 360.0f;
	return yaww;
}

void M::RotateCenter(const ImVec2& vecCenter, const float flAngle, ImVec2* pOutPoint)
{
	const float flSin = std::sinf(M_DEG2RAD(flAngle));
	const float flCos = std::cosf(M_DEG2RAD(flAngle));

	pOutPoint->x -= vecCenter.x;
	pOutPoint->y -= vecCenter.y;

	const float x = pOutPoint->x * flCos - pOutPoint->y * flSin;
	const float y = pOutPoint->x * flSin + pOutPoint->y * flCos;

	pOutPoint->x = x + vecCenter.x;
	pOutPoint->y = y + vecCenter.y;
}

float M::Approach(float target, float value, float speed) {
	float delta = target - value;

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}

Vector M::Approach(Vector target, Vector value, float speed) {

	float delta = target.DistTo(value);

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}

//float M::ApproachAngle(float target, float value, float speed) {
//
//	target = anglemod(target);
//	value = anglemod(value);
//
//	float delta = target - value;
//
//	if (speed < 0)
//		speed = -speed;
//
//	if (delta < -180)
//		delta += 360;
//	else if (delta > 180)
//		delta -= 360;
//
//	if (delta > speed)
//		value += speed;
//	else if (delta < -speed)
//		value -= speed;
//	else
//		value = target;
//
//	return value;
//}