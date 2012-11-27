//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

FORCE_INLINE
BaseArea::BaseArea ()
{
	;
}

FORCE_INLINE
BaseArea::BaseArea (const UPoint3D& min, const UPoint3D& max)
	: min_ (min), max_ (max)
{
	;
}

FORCE_INLINE
BaseArea::BaseArea (const BaseArea& otherArea)
	: min_ (otherArea.min_), max_ (otherArea.max_)
{
	;
}

FORCE_INLINE
BaseArea::~BaseArea ()
{
	;
}

FORCE_INLINE bool
BaseArea::operator==( const BaseArea& otherArea ) const
{
	if( min_ != otherArea.min_ )
		return false;

	if( max_ != otherArea.max_ )
		return false;

	return true;
}

FORCE_INLINE bool
BaseArea::operator!=( const BaseArea& otherArea ) const
{
	return !(*this == otherArea);
}

FORCE_INLINE const UPoint3D&
BaseArea::GetMin () const
{
	return min_;
}

FORCE_INLINE const UPoint3D&
BaseArea::GetMax() const
{
	return max_;
}

FORCE_INLINE UPoint3D
BaseArea::GetRandPointInArea( ) const
{
	return UPoint3D( GetRandX(), GetRandY(), GetRandZ() );
}

FORCE_INLINE UPoint3D
BaseArea::GetCenterPoint() const
{
	return UPoint3D (GetCenterX (), GetCenterY (), GetCenterZ ());
}

FORCE_INLINE float32
BaseArea::GetCenterPointDistance( const BaseArea& other ) const
{
	return GetCenterPoint().GetDistance( other.GetCenterPoint() );
}

FORCE_INLINE float32
BaseArea::GetCenterPointDistanceFast( const BaseArea& other ) const
{
	return GetCenterPoint().GetDistance( other.GetCenterPoint() );
}

FORCE_INLINE uint32
BaseArea::GetLengthX() const
{
	Assert (max_.x >= min_.x);
	return max_.x - min_.x + 1;
}

FORCE_INLINE uint32
BaseArea::GetLengthY() const
{
	Assert(max_.y >= min_.y);
	return max_.y - min_.y + 1;
}

FORCE_INLINE uint32
BaseArea::GetLengthZ() const
{
	Assert(max_.z >= min_.z);
	return max_.z - min_.z + 1;
}

FORCE_INLINE uint32
BaseArea::GetCenterX() const
{
	return min_.x + GetLengthX() / 2;
}

FORCE_INLINE uint32
BaseArea::GetCenterY () const
{
	return min_.y + GetLengthY() / 2;
}

FORCE_INLINE uint32
BaseArea::GetCenterZ () const
{
	return min_.z + GetLengthZ() / 2;
}

FORCE_INLINE uint32
BaseArea::GetRandX( ) const
{
	return min_.x + rand() % GetLengthX();
}

FORCE_INLINE uint32
BaseArea::GetRandY( ) const
{
	return min_.y + rand() % GetLengthY();
}

FORCE_INLINE uint32
BaseArea::GetRandZ( ) const
{
	return min_.z + rand() % GetLengthZ();
}

FORCE_INLINE void
BaseArea::SetMin (const UPoint3D& point)
{
	min_.x = point.x;
	min_.y = point.y;
	min_.z = point.z;
}

FORCE_INLINE void
BaseArea::SetMin (uint32 x, uint32 y, uint32 z)
{
	min_.x = x;
	min_.y = y;
	min_.z = z;
}

FORCE_INLINE void
BaseArea::SetMax (const UPoint3D& point)
{
	max_.x = point.x;
	max_.y = point.y;
	max_.z = point.z;
}

FORCE_INLINE void
BaseArea::SetMax (uint32 x, uint32 y, uint32 z)
{
	max_.x = x;
	max_.y = y;
	max_.z = z;
}

FORCE_INLINE void
BaseArea::SetMinX (uint32 x)
{
	min_.x = x;
}

FORCE_INLINE void
BaseArea::SetMinY (uint32 y)
{
	min_.y = y;
}

FORCE_INLINE void
BaseArea::SetMinZ (uint32 z)
{
	min_.z = z;
}

FORCE_INLINE void
BaseArea::SetMaxX (uint32 x)
{
	max_.x = x;
}

FORCE_INLINE void
BaseArea::SetMaxY (uint32 y)
{
	max_.y = y;
}

FORCE_INLINE void
BaseArea::SetMaxZ (uint32 z)
{
	max_.z = z;
}