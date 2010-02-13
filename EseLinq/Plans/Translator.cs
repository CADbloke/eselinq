using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;

using EseObjects;

namespace EseLinq.Plans
{
	/// <summary>
	/// ToPlans from a LINQ Expression to an EseLinq Plan.
	/// </summary>
	class Translator
	{
		internal struct Upstream
		{
			internal Dictionary<string, object> env;
			internal Plan context;

			///<summary>use this constructor when about to make modifications; copies mutable members</summary>
			internal Upstream(Upstream from)
			{
				this.env = new Dictionary<string,object>(from.env);
				this.context = from.context;
			}

			internal void Init()
			{
				env = new Dictionary<string, object>(16);
			}
		}

		internal struct Downstream<T>
		{
			internal T result;
			///<summary>either a ScalarPlan or a Plan representing the expression value</summary>
			internal object value_plan;
			/// <summary>Plan used for scrolling, if applicable</summary>
			internal Plan plan;

			internal Downstream(T result, object value_plan, Plan plan)
			{
				this.result = result;
				this.value_plan = value_plan;
				this.plan = plan;
			}

			internal Downstream(Downstream<T> downs, T result)
			{
				this.result = result;
				this.value_plan = downs.value_plan;
				this.plan = downs.plan;
			}

			internal Downstream(T result)
			{
				this.result = result;
				this.value_plan = result;
				this.plan = null;
			}

			internal Downstream<T> WithResult(T result)
			{
				Downstream<T> r = this;
				r.result = result;
				return r;
			}

			internal Downstream<T> WithValueResult(T result)
			{
				Downstream<T> r = this;
				r.value_plan = result;
				r.result = result;
				return r;
			}

			//internal Downstream<T> Merge(Downstream<T> other)
			//{
			//    //nothing to merge at this point
			//    return this;
			//}

			internal Downstream<T> Merge<U>(Downstream<U> other)
			{
				//nothing to merge at this point
				return this;
			}
		}

		internal static Downstream<Plan> DownstreamBasePlan(Plan plan)
		{
			return new Downstream<Plan>(plan, plan, plan);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(UnaryExpression exp, Upstream ups)
		{
			switch(exp.NodeType)
			{
			//don't care about quotes
			case ExpressionType.Quote:
				return ToScalarPlan(exp.Operand, ups);
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(BinaryExpression exp, Upstream ups)
		{
			switch(exp.NodeType)
			{
			case ExpressionType.Equal:
				{
					var left = ToScalarPlan(exp.Left, ups);
					var right = ToScalarPlan(exp.Right, ups);

					return left.Merge(right).WithValueResult(new BinaryCalc(exp.NodeType, left.result, right.result));
				}
			
			default:
				throw IDontKnowWhatToDoWithThis(exp);
			}
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(ConditionalExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(ConstantExpression exp, Upstream ups)
		{
			return new Downstream<ScalarPlan>(new Constant(exp.Value));
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(InvocationExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(LambdaExpression exp, Upstream ups)
		{
			//copy value for upstream use
			ups = new Upstream(ups);

			Plan bodyp = ups.context;

			ups.env.Add(exp.Parameters[0].Name, bodyp);

			return ToScalarPlan(exp.Body, ups);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(ListInitExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(MemberExpression exp, Upstream ups)
		{
			var parm = (ParameterExpression)exp.Expression;
			var field = (FieldInfo)exp.Member;
			var tplan = (Plan)ups.env[parm.Name];

			var result = new Retrieve(tplan, field.Name, field.FieldType);

			return new Downstream<ScalarPlan>(result);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(MemberInitExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(MethodCallExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(NewArrayExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(NewExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(ParameterExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(TypeBinaryExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}


		internal static Downstream<Plan> ToPlan(UnaryExpression exp, Upstream ups)
		{
			switch(exp.NodeType)
			{
			//don't care about quotes
			case ExpressionType.Quote:
				return ToPlan(exp.Operand, ups);
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(BinaryExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(ConditionalExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(ConstantExpression exp, Upstream ups)
		{
			var planned = exp.Value as PrePlanned;
			if(planned != null)
				return DownstreamBasePlan(planned.plan);

			return ToPlan((Expression)exp.Value, ups);
		}

		internal static Downstream<Plan> ToPlan(InvocationExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(LambdaExpression exp, Upstream ups)
		{
			//assuming one parameter

			//copy value for upstream use
			ups = new Upstream(ups);

			var body = ToPlan(exp.Body, ups);

			ups.env.Add(exp.Parameters[0].Name, body.result);

			return body;
		}

		internal static Downstream<Plan> ToPlan(ListInitExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(MemberExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(MemberInitExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(MethodCallExpression exp, Upstream ups)
		{
			switch(exp.Method.Name)
			{
			case "Where":
				{
					var data = ToPlan(exp.Arguments[0], ups);
					ups.context = data.plan;
					var pred = ToScalarPlan(exp.Arguments[1], ups);
					var body = new Filter(data.result, pred.result);

					data = data.Merge(pred);
					data.plan = body;

					return data;
				}
			case "Select":
				{
					var data = ToPlan(exp.Arguments[0], ups);
					ups.context = data.plan;
					var body = ToScalarPlan(exp.Arguments[1], ups);

					data = data.Merge(body);
					data.value_plan = body.value_plan;

					return data;
				}
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(NewArrayExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(NewExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(ParameterExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(TypeBinaryExpression exp, Upstream ups)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<ScalarPlan> ToScalarPlan(Expression exp, Upstream ups)
		{
			{
				var tyexp = exp as UnaryExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as BinaryExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as TypeBinaryExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as ConstantExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as ConditionalExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as MemberExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as MethodCallExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as LambdaExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as NewExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as MemberInitExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as ListInitExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as NewArrayExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}
			{
				var tyexp = exp as InvocationExpression;
				if(tyexp != null)
					return ToScalarPlan(tyexp, ups);
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Downstream<Plan> ToPlan(Expression exp, Upstream ups)
		{
			{
				var tyexp = exp as UnaryExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}
			{
				var tyexp = exp as BinaryExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}
			{
				var tyexp = exp as TypeBinaryExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}

			{
				var tyexp = exp as ConstantExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}
			{
				var tyexp = exp as ConditionalExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}
			{
				var tyexp = exp as MemberExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}
			{
				var tyexp = exp as MethodCallExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}
			{
				var tyexp = exp as LambdaExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}
			{
				var tyexp = exp as NewExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}
			{
				var tyexp = exp as MemberInitExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}
			{
				var tyexp = exp as ListInitExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}
			{
				var tyexp = exp as NewArrayExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}
			{
				var tyexp = exp as InvocationExpression;
				if(tyexp != null)
					return ToPlan(tyexp, ups);
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Exception IDontKnowWhatToDoWithThis(Expression exp)
		{
			return new ArgumentException("Unknown expression " + exp.Type + ":" + exp.NodeType);
		}
	}
}
