#pragma once

namespace LF
{
	//! 数学常量

	const float MATH_PI			= 3.14159265358979323846f;
	const float MATH_PI_OVER2	= 1.57079632679489661923f;
	const float MATH_PI_OVER4	= 0.785398163397448309616f;
	const float MATH_1OVER_PI	= 0.318309886183790671538f;
	const float MATH_2OVER_PI	= 0.636619772367581343076f;

	const float	MATH_2PI		= 6.28318530717958647692f;

	//! 度数转弧度
	const float DEGTORAD = MATH_PI / 180.0f;
	//! 弧度转度数
	const float RADTODEG = 180.0f / MATH_PI;

	//! 浮点数比较容差
	const float FLOAT_TOLERANCE = 0.00005f;

	//! 检查浮点数是否相等
	inline bool equals_f(float a, float b)
	{
		return (a + FLOAT_TOLERANCE > b) && (a - FLOAT_TOLERANCE < b);
	}


	//! 反平方根
	__forceinline float reciprocal_squareroot(float x)
	{
		__asm
		{
			movss	xmm0, x
			rsqrtss xmm0, xmm0
			movss	x, xmm0
		}
		return x;
	}

	//! 尺寸类摸版
	template <class T>
	class dimension
	{
	public:

		dimension()
			: w(0), h(0) {};

		dimension(T width, T height)
			: w(width), h(height) {};

		dimension(const dimension<T>  &other)
			: w(other.w), h(other.h) {};


		bool operator == (const dimension<T>& other) const
		{
			return w == other.w && h == other.h;
		}


		bool operator != (const dimension<T>& other) const
		{
			return w != other.w || h != other.h;
		}

		const dimension<T>& operator=(const dimension<T>& other) 
		{
			w = other.w;
			h = other.h;
			return *this;
		}

		dimension<T> operator/(T scale) 
		{
			return dimension<T>(w/scale, h/scale);
		}

		dimension<T> operator*(T scale) 
		{
			return dimension<T>(w*scale, h*scale);
		}

		T w, h;
	};

	typedef dimension<float> size_f;
	typedef dimension<int>   size_i;


	//! 位置类摸版
	template <class T>
	class position
	{
	public:
		position(T x, T y)
			: X(x), Y(y) {};


		position()
			: X(0), Y(0) {};


		position(const position<T>& other)
			: X(other.X), Y(other.Y) {};


		bool operator == (const position<T>& other) const
		{
			return X == other.X && Y == other.Y;
		}


		bool operator != (const position<T>& other) const
		{
			return X != other.X || Y != other.Y;
		}

		const position<T>& operator+=(const position<T>& other)
		{
			X += other.X;
			Y += other.Y;
			return *this;
		}

		const position<T>& operator-=(const position<T>& other)
		{
			X -= other.X;
			Y -= other.Y;
			return *this;
		}

		const position<T>& operator+=(const dimension<T>& other)
		{
			X += other.w;
			Y += other.h;
			return *this;
		}

		const position<T>& operator-=(const dimension<T>& other)
		{
			X -= other.w;
			Y -= other.h;
			return *this;
		}

		position<T> operator-(const position<T>& other) const
		{
			return position<T>(X-other.X, Y-other.Y);
		}

		position<T> operator+(const position<T>& other) const
		{
			return position<T>(X+other.X, Y+other.Y);
		}

		position<T> operator*(const position<T>& other) const
		{
			return position<T>(X*other.X, Y*other.Y);
		}

		position<T> operator*(const T& scalar) const
		{
			return position<T>(X*scalar, Y*scalar);
		}

		position<T> operator+(const dimension<T>& other) const
		{
			return position<T>(X+other.w, Y+other.h);
		}

		position<T> operator-(const dimension<T>& other) const
		{
			return position<T>(X-other.w, Y-other.h);
		}

		const position<T>& operator=(const position<T>& other) 
		{
			X = other.X;
			Y = other.Y;
			return *this;
		}

		T  X, Y;
	};


	typedef position<float> pos_f;
	typedef position<int> pos_i;


	//! 矩形类摸版
	template <class T>
	class rectangle
	{
	public:

		rectangle()
			: UpperLeftCorner(0,0), LowerRightCorner(0,0) {};


		rectangle(T x, T y, T x2, T y2)
			: UpperLeftCorner(x,y), LowerRightCorner(x2,y2) {};


		rectangle(const position<T>& upperLeft, const position<T>& lowerRight)
			: UpperLeftCorner(upperLeft), LowerRightCorner(lowerRight) {};

		rectangle(const rectangle<T>& other)
			: UpperLeftCorner(other.UpperLeftCorner), LowerRightCorner(other.LowerRightCorner) {};

		rectangle(const position<T>& pos, const dimension<T>& size)
			: UpperLeftCorner(pos), LowerRightCorner(pos.X + size.w, pos.Y + size.h) {};


		rectangle<T> operator+(const position<T>& pos) const
		{
			rectangle<T> ret(*this);
			ret.UpperLeftCorner += pos;
			ret.LowerRightCorner += pos;
			return ret;
		}

		const rectangle<T>& operator+=(const position<T>& pos)
		{
			UpperLeftCorner += pos;
			LowerRightCorner += pos;
			return *this;
		}

		rectangle<T> operator-(const position<T>& pos) const
		{
			rectangle<T> ret(*this);
			ret.UpperLeftCorner -= pos;
			ret.LowerRightCorner -= pos;
			return ret;
		}

		const rectangle<T>& operator-=(const position<T>& pos)
		{
			UpperLeftCorner -= pos;
			LowerRightCorner -= pos;
			return *this;
		}

		bool operator == (const rectangle<T>& other) const
		{
			return (UpperLeftCorner == other.UpperLeftCorner &&
				LowerRightCorner == other.LowerRightCorner);
		}


		bool operator != (const rectangle<T>& other) const
		{
			return (UpperLeftCorner != other.UpperLeftCorner ||
				LowerRightCorner != other.LowerRightCorner);
		}

		const rectangle<T>& operator = (const rectangle<T>& other)
		{
			UpperLeftCorner = other.UpperLeftCorner;
			LowerRightCorner = other.LowerRightCorner;
			return *this;
		}

		//! 检查点是否在矩形区域中
		bool isPointInside(const position<T>& pos) const
		{
			return (UpperLeftCorner.X <= pos.X &&
				UpperLeftCorner.Y <= pos.Y &&
				LowerRightCorner.X >= pos.X &&
				LowerRightCorner.Y >= pos.Y);
		}

		bool isRectInside(const rectangle<T>& other) const
		{
			return isPointInside(other.UpperLeftCorner) &&
				isPointInside(other.LowerRightCorner);
		}

		//! 碰撞检测
		bool isRectCollided(const rectangle<T>& other) const
		{
			return (LowerRightCorner.Y > other.UpperLeftCorner.Y &&
				UpperLeftCorner.Y < other.LowerRightCorner.Y &&
				LowerRightCorner.X > other.UpperLeftCorner.X &&
				UpperLeftCorner.X < other.LowerRightCorner.X);
		}

		//! 剪裁
		void clipAgainst(const rectangle<T>& other) 
		{
			if (other.LowerRightCorner.X < LowerRightCorner.X)
				LowerRightCorner.X = other.LowerRightCorner.X;
			if (other.LowerRightCorner.Y < LowerRightCorner.Y)
				LowerRightCorner.Y = other.LowerRightCorner.Y;

			if (other.UpperLeftCorner.X > UpperLeftCorner.X)
				UpperLeftCorner.X = other.UpperLeftCorner.X;
			if (other.UpperLeftCorner.Y > UpperLeftCorner.Y)
				UpperLeftCorner.Y = other.UpperLeftCorner.Y;

			// 校正
			if (UpperLeftCorner.Y > LowerRightCorner.Y)
				UpperLeftCorner.Y = LowerRightCorner.Y;
			if (UpperLeftCorner.X > LowerRightCorner.X)
				UpperLeftCorner.X = LowerRightCorner.X;
		}

		//! 重置
		void reset()
		{
			LowerRightCorner.X = LowerRightCorner.Y = UpperLeftCorner.X = UpperLeftCorner.Y = 0;
		}

		//! 获得宽度
		T getWidth() const
		{
			return LowerRightCorner.X - UpperLeftCorner.X;
		}

		//! 获得高度
		T getHeight() const
		{
			return LowerRightCorner.Y - UpperLeftCorner.Y;
		}

		//! 修正，当右下角小于左上角时，两者交换
		void repair()
		{
			if (LowerRightCorner.X < UpperLeftCorner.X)
			{
				T t = LowerRightCorner.X;
				LowerRightCorner.X = UpperLeftCorner.X;
				UpperLeftCorner.X = t;
			}

			if (LowerRightCorner.Y < UpperLeftCorner.Y)
			{
				T t = LowerRightCorner.Y;
				LowerRightCorner.Y = UpperLeftCorner.Y;
				UpperLeftCorner.Y = t;
			}
		}

		//! 检查数据是否合法
		bool isValid() const
		{
			// thx to jox for a correction to this method

			T xd = LowerRightCorner.X - UpperLeftCorner.X;
			T yd = LowerRightCorner.Y - UpperLeftCorner.Y;

			return !(xd < 0 || yd < 0 || (xd == 0 && yd == 0));
		}

		//! 获取中心位置
		position<T> getCenter() const
		{
			return position<T>((UpperLeftCorner.X + LowerRightCorner.X) / 2,
				(UpperLeftCorner.Y + LowerRightCorner.Y) / 2);
		}

		//! 获取尺寸
		dimension<T> getSize() const
		{
			return dimension<T>(getWidth(), getHeight());
		}


		//! 添加扩张点,当点在矩形区域外时，矩形将自动增长将点包含在内
		void addInternalPoint(const position<T>& p)
		{
			addInternalPoint(p.X, p.Y);
		}

		//! 同上
		void addInternalPoint(T x, T y)
		{
			if (x>LowerRightCorner.X) LowerRightCorner.X = x;
			if (y>LowerRightCorner.Y) LowerRightCorner.Y = y;

			if (x<UpperLeftCorner.X) UpperLeftCorner.X = x;
			if (y<UpperLeftCorner.Y) UpperLeftCorner.Y = y;
		}



		position<T> UpperLeftCorner;
		position<T> LowerRightCorner;

	};
	typedef rectangle<float> rect_f;
	typedef rectangle<int>   rect_i;


	//! 2d向量摸版
	template <class T>
	class vector_t
	{
	public:

		vector_t() : X(0), Y(0) {};
		vector_t(T nx, T ny) : X(nx), Y(ny) {};
		vector_t(const vector_t<T>& other) : X(other.X), Y(other.Y) {};

		// operators

		vector_t<T> operator-() const { return vector_t<T>(-X, -Y);   }

		vector_t<T>& operator=(const vector_t<T>& other)	{ X = other.X; Y = other.Y; return *this; }

		vector_t<T> operator+(const vector_t<T>& other) const { return vector_t<T>(X + other.X, Y + other.Y);	}
		vector_t<T>& operator+=(const vector_t<T>& other)	{ X+=other.X; Y+=other.Y; return *this; }

		vector_t<T> operator-(const vector_t<T>& other) const { return vector_t<T>(X - other.X, Y - other.Y);	}
		vector_t<T>& operator-=(const vector_t<T>& other)	{ X-=other.X; Y-=other.Y; return *this; }

		vector_t<T> operator*(const vector_t<T>& other) const { return vector_t<T>(X * other.X, Y * other.Y);	}
		vector_t<T>& operator*=(const vector_t<T>& other)	{ X*=other.X; Y*=other.Y; return *this; }
		vector_t<T> operator*(const T v) const { return vector_t<T>(X * v, Y * v);	}
		vector_t<T>& operator*=(const T v) { X*=v; Y*=v; return *this; }

		vector_t<T> operator/(const vector_t<T>& other) const { return vector_t<T>(X / other.X, Y / other.Y);	}
		vector_t<T>& operator/=(const vector_t<T>& other)	{ X/=other.X; Y/=other.Y; return *this; }
		vector_t<T> operator/(const T v) const { return vector_t<T>(X / v, Y / v);	}
		vector_t<T>& operator/=(const T v) { X/=v; Y/=v; return *this; }

		bool operator<=(const vector_t<T>&other) const { return X<=other.X && Y<=other.Y; }
		bool operator>=(const vector_t<T>&other) const { return X>=other.X && Y>=other.Y; }

		bool operator<(const vector_t<T>&other) const { return X<other.X && Y<other.Y; }
		bool operator>(const vector_t<T>&other) const { return X>other.X && Y>other.Y; }

		bool operator==(const vector_t<T>& other) const { return other.X==X && other.Y==Y; }
		bool operator!=(const vector_t<T>& other) const { return other.X!=X || other.Y!=Y; }

		// functions

		//! 检查两个向量是否等价
		bool equals(const vector_t<T>& other) const
		{
			return equals_f(X, other.X) && equals_f(Y, other.Y);
		}

		void set(T nx, T ny) { X=nx; Y=ny; }
		void set(const vector_t<T>& p) { X=p.X; Y=p.Y;}

		//! 获取长度
		double getLength() const { return sqrt(X*X + Y*Y); }

		//! 获取长度的平方值
		T getLengthSQ() const { return X*X + Y*Y; }

		//! 点积
		T dotProduct(const vector_t<T>& other) const
		{
			return X*other.X + Y*other.Y;
		}

		//! 获取两向量间距离
		double getDistanceFrom(const vector_t<T>& other) const
		{
			return vector_t<T>(X - other.X, Y - other.Y).getLength();
		}

		//! 获取两向量间距离的平方
		T getDistanceFromSQ(const vector_t<T>& other) const
		{
			return vector_t<T>(X - other.X, Y - other.Y).getLengthSQ();
		}

		//! 围绕某点旋转
		void rotateBy(float degrees, const vector_t<T>& center)
		{
			degrees *= DEGTORAD;
			T cs = (T)cos(degrees);
			T sn = (T)sin(degrees);

			X -= center.X;
			Y -= center.Y;

			set(X*cs - Y*sn, X*sn + Y*cs);

			X += center.X;
			Y += center.Y;
		}

		vector_t<T>& normalize()
		{
			T l = reciprocal_squareroot ( X*X + Y*Y );
			/*
			T l = (T)getLength();
			if (l == 0)
			return *this;

			l = (T)1.0 / l;
			*/
			X *= l;
			Y *= l;
			return *this;
		}

		double getAngleTrig() const
		{
			if (X == 0.0)
				return Y < 0.0 ? 270.0 : 90.0;
			else
				if (Y == 0)
					return X < 0.0 ? 180.0 : 0.0;

			if ( Y > 0.0)
				if (X > 0.0)
					return atan(Y/X) * RADTODEG;
				else
					return 180.0-atan(Y/-X) * RADTODEG;
			else
				if (X > 0.0)
					return 360.0-atan(-Y/X) * RADTODEG;
				else
					return 180.0+atan(-Y/-X) * RADTODEG;
		} 

		inline double getAngle() const
		{
			if (Y == 0.0)  // corrected thanks to a suggestion by Jox
				return X < 0.0 ? 180.0 : 0.0; 
			else if (X == 0.0) 
				return Y < 0.0 ? 90.0 : 270.0;

			double tmp = Y / getLength();
			tmp = atan(sqrt(1 - tmp*tmp) / tmp) * RADTODEG;

			if (X>0.0 && Y>0.0)
				return tmp + 270;
			else
				if (X>0.0 && Y<0.0)
					return tmp + 90;
				else
					if (X<0.0 && Y<0.0)
						return 90 - tmp;
					else
						if (X<0.0 && Y>0.0)
							return 270 - tmp;

			return tmp;
		}

		//! 返回与另一个向量的插值
		inline double getAngleWith(const vector_t<T>& b) const
		{
			double tmp = X*b.X + Y*b.Y;

			if (tmp == 0.0)
				return 90.0;

			tmp = tmp / sqrt((X*X + Y*Y) * (b.X*b.X + b.Y*b.Y));
			if (tmp < 0.0) tmp = -tmp;

			return atan(sqrt(1 - tmp*tmp) / tmp) * RADTODEG;
		}

		//! 检查是否在两点之间
		bool isBetweenPoints(const vector_t<T>& begin, const vector_t<T>& end) const
		{
			T f = (end - begin).getLengthSQ();
			return getDistanceFromSQ(begin) < f && 
				getDistanceFromSQ(end) < f;
		}

		//! 返回插值
		vector_t<T> getInterpolated(const vector_t<T>& other, float d) const
		{
			T inv = (T) 1.0 - d;
			return vector_t<T>(other.X*inv + X*d, other.Y*inv + Y*d);
		}

		//! 2次插值
		vector_t<T> getInterpolated_quadratic(const vector_t<T>& v2, const vector_t<T>& v3, const T d) const
		{
			// this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
			const T inv = (T) 1.0 - d;
			const T mul0 = inv * inv;
			const T mul1 = (T) 2.0 * d * inv;
			const T mul2 = d * d;

			return vector_t<T> ( X * mul0 + v2.X * mul1 + v3.X * mul2,
				Y * mul0 + v2.Y * mul1 + v3.Y * mul2
				);
		}

		//! 差值，t = 0.0 - 0.1 
		void interpolate(const vector_t<T>& a, const vector_t<T>& b, const float t)
		{
			X = b.X + ( ( a.X - b.X ) * t );
			Y = b.Y + ( ( a.Y - b.Y ) * t );
		}

		// member variables
		T X, Y;
	};

	typedef vector_t<float> vector_f;
	typedef vector_t<int>   vector_i;

	template<class S, class T> vector_t<T> operator*(const S scalar, const vector_t<T>& vector) { return vector*scalar; }


	//! 2d线摸版
	template <class T>
	class line_t
	{
	public:

		line_t() : start(0,0), end(1,1) {};
		line_t(T xa, T ya, T xb, T yb) : start(xa, ya), end(xb, yb) {};
		line_t(const vector_t<T>& start, const vector_t<T>& end) : start(start), end(end) {};
		line_t(const line_t<T>& other) : start(other.start), end(other.end) {};

		// operators

		line_t<T> operator+(const vector_t<T>& point) const { return line_t<T>(start + point, end + point); };
		line_t<T>& operator+=(const vector_t<T>& point) { start += point; end += point; return *this; };

		line_t<T> operator-(const vector_t<T>& point) const { return line_t<T>(start - point, end - point); };
		line_t<T>& operator-=(const vector_t<T>& point) { start -= point; end -= point; return *this; };

		bool operator==(const line_t<T>& other) const { return (start==other.start && end==other.end) || (end==other.start && start==other.end);};
		bool operator!=(const line_t<T>& other) const { return !(start==other.start && end==other.end) || (end==other.start && start==other.end);};

		// functions

		void setLine(const T& xa, const T& ya, const T& xb, const T& yb){start.set(xa, ya); end.set(xb, yb);}
		void setLine(const vector_t<T>& nstart, const vector_t<T>& nend){start.set(nstart); end.set(nend);}
		void setLine(const line_t<T>& line){start.set(line.start); end.set(line.end);}

		double getLength() const { return start.getDistanceFrom(end); };

		T getLengthSQ() const { return start.getDistanceFromSQ(end); };

		vector_t<T> getMiddle() const
		{
			return (start + end) * (T)0.5;
		}

		vector_t<T> getVector() const { return vector_t<T>(start.X - end.X, start.Y - end.Y); };

		//! 检查是否与直线交叉,是则改写传入的交点.
		bool intersectWith(const line_t<T>& l, vector_t<T>& out) const
		{
			bool found=false;

			float a1,a2,b1,b2;

			// calculate slopes, deal with infinity
			if (end.X-start.X == 0)
				b1 = (float)1e+10;
			else
				b1 = (end.Y-start.Y)/(end.X-start.X);
			if (l.end.X-l.start.X == 0)
				b2 = (float)1e+10;
			else
				b2 = (l.end.Y-l.start.Y)/(l.end.X-l.start.X);

			// calculate position
			a1 = start.Y   - b1 *  start.X;
			a2 = l.start.Y - b2 * l.start.X;
			out.X = - (a1-a2)/(b1-b2);
			out.Y = a1 + b1*out.X;

			// did the lines cross?
			if (	(start.X-out.X) *(out.X-end.X)	 >= -FLOAT_TOLERANCE &&
				(l.start.X-out.X)*(out.X-l.end.X)>= -FLOAT_TOLERANCE &&
				(start.Y-out.Y)  *(out.Y-end.Y)  >= -FLOAT_TOLERANCE &&
				(l.start.Y-out.Y)*(out.Y-l.end.Y)>= -FLOAT_TOLERANCE )
			{
				found = true;
			}
			return found;
		}

		//! 返回单位向量
		vector_t<T> getUnitVector()
		{
			T len = (T)(1.0 / getLength());
			return vector_t<T>((end.X - start.X) * len, (end.Y - start.Y) * len);
		}

		double getAngleWith(const line_t<T>& l)
		{
			vector_t<T> vect = getVector();
			vector_t<T> vect2 = l.getVector();
			return vect.getAngleWith(vect2);
		}

		//! 返回点和线位置的描述, 0 表示在线上， <0 表示
		//! 在线左边，>0表示在线右边.
		T getPointOrientation(const vector_t<T>& point)
		{
			return ( (end.X   - start.X) * (point.Y - start.Y) - 
				(point.X - start.X) * (end.Y   - start.Y) );
		}


		//! 检查该点是否在线上
		bool isPointOnLine(const vector_t<T>& point)
		{
			T d = getPointOrientation(point);
			return (d == 0 && point.isBetweenPoints(start, end));
		}


		//! 是否在线段内
		bool isPointBetweenStartAndEnd(const vector_t<T>& point) const
		{
			return point.isBetweenPoints(start, end);
		}

		//! 返回线上离该点最近的点
		vector_t<T> getClosestPoint(const vector_t<T>& point) const
		{
			vector_t<T> c = point - start;
			vector_t<T> v = end - start;
			T d = (T)v.getLength();
			v /= d;
			T t = v.dotProduct(c);

			if (t < (T)0.0) return start;
			if (t > d) return end;

			v *= t;
			return start + v;
		}

		// member variables

		vector_t<T> start;
		vector_t<T> end;
	};

	typedef line_t<float> line_f;
	typedef line_t<int>   line_i;
}