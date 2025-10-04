/*********************************************************************
	varCheck.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef VARCHECKCLASS_H
#define VARCHECKCLASS_H

// classes
template <class varType> class varCheckClass																												
{																																							
public:																																						
	/* operators */																																			
	inline varCheckClass<varType>	 				operator -  			(const varCheckClass<varType>& right)		{	varCheckClass<varType> myClass(this->var - right.var); return myClass;	};	
	inline varCheckClass<varType>  					operator +  			(const varCheckClass<varType>& right)		{	varCheckClass<varType> myClass(this->var + right.var); return myClass;	};				
	inline varCheckClass<varType> 					operator *  			(const varCheckClass<varType>& right)		{	varCheckClass<varType> myClass(this->var * right.var); return myClass;	};					
	inline varCheckClass<varType>  					operator /  			(const varCheckClass<varType>& right)		{	varCheckClass<varType> myClass(this->var / right.var); return myClass;	};				
	inline varCheckClass<varType>  					operator %  			(const varCheckClass<varType>& right)		{	varCheckClass<varType> myClass(this->var % right.var); return myClass;	};					
	inline varCheckClass<varType> & 				operator -=  			(const varCheckClass<varType>& right)		{	(this->var -=  right.var);	return *this;	};					
	inline varCheckClass<varType> & 				operator +=  			(const varCheckClass<varType>& right)		{	(this->var +=  right.var);	return *this;	};					
	inline varCheckClass<varType> & 				operator ++ 			()											{	(++this->var);				return *this;	};					
	inline varCheckClass<varType> & 				operator -- 			()											{	(--this->var);				return *this;	};					
	inline varCheckClass<varType> & 				operator ++ 			(int)										{	(this->var++);				return *this;	};					
	inline varCheckClass<varType> & 				operator -- 			(int)										{	(this->var--);				return *this;	};					
	inline bool										operator <  			(const varCheckClass<varType>& right)		{	return (this->var <  right.var);			};					
	inline bool										operator >  			(const varCheckClass<varType>& right)		{	return (this->var >  right.var);			};					
	inline bool										operator != 			(const varCheckClass<varType>& right)		{	return (this->var != right.var);			};					
	inline bool										operator == 			(const varCheckClass<varType>& right)		{	return (this->var == right.var);			};					
	inline bool										operator >= 			(const varCheckClass<varType>& right)		{	return (this->var >= right.var);			};					
	inline bool										operator <= 			(const varCheckClass<varType>& right)		{	return (this->var <= right.var);			};					
	void											operator =				(const varCheckClass<varType>& right)		{	this->var = right.var;						};
	varType *										operator &				()											{	return &this->var;							};					
																																							
	/* Dieser Typumwandlungsoperator könnte problematisch sein, da er zusammen mit dem Konstruktor eine ungewünschte Typumwandlung ermöglicht.*/			
	//												operator unsigned int	()									const	{	return this->var;							};					
																																									
	/* constructors */																																				
	varCheckClass<varType>													(const varType _var) 						{	var = _var;									};					
	varCheckClass<varType>													()						  					{	var = 0;									};					
																																							
	/* the single variable */																																
	varType	var;																																			
};

template <class varType> class varCheckTypeCaster																												
{																																							
public:																																						
	/* operators */																																			
	inline varCheckTypeCaster<varType>	 			operator -  			(const varCheckTypeCaster<varType>& right)		{	varCheckTypeCaster<varType> myClass(this->var - right.var); return myClass;	};	
	inline varCheckTypeCaster<varType>  			operator +  			(const varCheckTypeCaster<varType>& right)		{	varCheckTypeCaster<varType> myClass(this->var + right.var); return myClass;	};				
	inline varCheckTypeCaster<varType> 				operator *  			(const varCheckTypeCaster<varType>& right)		{	varCheckTypeCaster<varType> myClass(this->var * right.var); return myClass;	};					
	inline varCheckTypeCaster<varType>  			operator /  			(const varCheckTypeCaster<varType>& right)		{	varCheckTypeCaster<varType> myClass(this->var / right.var); return myClass;	};				
	inline varCheckTypeCaster<varType>  			operator %  			(const varCheckTypeCaster<varType>& right)		{	varCheckTypeCaster<varType> myClass(this->var % right.var); return myClass;	};					
	inline varCheckTypeCaster<varType> & 			operator -=  			(const varCheckTypeCaster<varType>& right)		{	(this->var -=  right.var);	return *this;	};					
	inline varCheckTypeCaster<varType> & 			operator +=  			(const varCheckTypeCaster<varType>& right)		{	(this->var +=  right.var);	return *this;	};					
	inline varCheckTypeCaster<varType> & 			operator ++ 			()												{	(++this->var);				return *this;	};					
	inline varCheckTypeCaster<varType> & 			operator -- 			()												{	(--this->var);				return *this;	};					
	inline varCheckTypeCaster<varType> & 			operator ++ 			(int)											{	(this->var++);				return *this;	};					
	inline varCheckTypeCaster<varType> & 			operator -- 			(int)											{	(this->var--);				return *this;	};					
	inline bool										operator <  			(const varCheckTypeCaster<varType>& right)		{	return (this->var <  right.var);			};					
	inline bool										operator >  			(const varCheckTypeCaster<varType>& right)		{	return (this->var >  right.var);			};					
	inline bool										operator != 			(const varCheckTypeCaster<varType>& right)		{	return (this->var != right.var);			};					
	inline bool										operator == 			(const varCheckTypeCaster<varType>& right)		{	return (this->var == right.var);			};					
	inline bool										operator >= 			(const varCheckTypeCaster<varType>& right)		{	return (this->var >= right.var);			};					
	inline bool										operator <= 			(const varCheckTypeCaster<varType>& right)		{	return (this->var <= right.var);			};					
	void											operator =				(const varCheckTypeCaster<varType>& right)		{	this->var = right.var;						};
	varType *										operator &				()												{	return &this->var;							};					
																																							
	/* Dieser Typumwandlungsoperator könnte problematisch sein, da er zusammen mit dem Konstruktor eine ungewünschte Typumwandlung ermöglicht.*/			
													operator unsigned int	()										const	{	return this->var;							};					
																																										
	/* constructors */																																					
	varCheckTypeCaster<varType>												(const varCheckClass<varType>& _class) 			{	var = _class.var;							};					
	varCheckTypeCaster<varType>												(const varType _var) 							{	var = _var;									};					
	varCheckTypeCaster<varType>												()						  						{	var = 0;									};					
																																							
	/* the single variable */																																
	varType	var;																																			
};

template <class varType, class indexType> class varCheckArray																												
{																																							
public:																																						
																																							
	/* operators */																																			
	inline varCheckArray<varType, indexType> & 		operator ++ 			()															{	(++this->p);				return *this;	};						
	inline varCheckArray<varType, indexType> & 		operator -- 			()															{	(--this->p);				return *this;	};						
	inline varCheckArray<varType, indexType> & 		operator ++ 			(int)														{	(this->p++);				return *this;	};						
	inline varCheckArray<varType, indexType> & 		operator -- 			(int)														{	(this->p--);				return *this;	};						
	inline bool										operator <  			(const varCheckArray<varType, indexType>& right)			{	return (this->p <  right.p);				};						
	inline bool										operator >  			(const varCheckArray<varType, indexType>& right)			{	return (this->p >  right.p);				};						
	inline bool										operator == 			(const varCheckArray<varType, indexType>& right)			{	return (this->p == right.p);				};						
	inline bool										operator >= 			(const varCheckArray<varType, indexType>& right)			{	return (this->p >= right.p);				};						
	inline bool										operator <= 			(const varCheckArray<varType, indexType>& right)			{	return (this->p <= right.p);				};						
	inline bool										operator != 			(const int&   right)										{	return (this->p != (varType*) right);		};						
	inline varType &								operator []				(const indexType& index)									{   return p[index.var];						};												
    inline void										operator =				(varType* right)											{   p = right;									};															
													operator void*			()													const	{   return (void*)this->p;						};												
																																														
	/* constructors */																																									
		   varCheckArray<varType, indexType>								()															{	p = 0;										};						
		   varCheckArray<varType, indexType>								(varType*  _p )												{	p = _p;										};						
																																													
	/* the single variable */																																
	varType	*	p;																																			
};

#endif