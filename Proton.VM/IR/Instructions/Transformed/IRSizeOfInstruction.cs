using Proton.LIR;
using LIRInstructions = Proton.LIR.Instructions;
using System;
using System.Collections.Generic;

namespace Proton.VM.IR.Instructions
{
	public sealed class IRSizeOfInstruction : IRInstruction
	{
		private IRType mType = null;
		public IRType Type { get { return mType; } private set { mType = value; } }

		public IRSizeOfInstruction(IRType pType) : base(IROpcode.SizeOf) { Type = pType; }

		public override void Linearize(Stack<IRStackObject> pStack)
		{
			IRLinearizedLocation value = new IRLinearizedLocation(this, IRLinearizedLocationType.SizeOf);
			value.SizeOf.Type = Type;
			Sources.Add(value);

			IRStackObject result = new IRStackObject();
			result.Type = ParentMethod.Assembly.AppDomain.System_UInt32;
			result.LinearizedTarget = new IRLinearizedLocation(this, IRLinearizedLocationType.Local);
			result.LinearizedTarget.Local.LocalIndex = AddLinearizedLocal(pStack, ParentMethod.Assembly.AppDomain.System_UInt32);
			Destination = new IRLinearizedLocation(this, result.LinearizedTarget);
			pStack.Push(result);
		}

		public override IRInstruction Clone(IRMethod pNewMethod) { return CopyTo(new IRSizeOfInstruction(Type), pNewMethod); }

		public override IRInstruction Transform() { return new IRMoveInstruction(this); }

		public override bool Resolved { get { return Type.Resolved; } }
		public override void Resolve()
		{
			base.Resolve();
			Type.Resolve(ref mType, ParentMethod.ParentType.GenericParameters, ParentMethod.GenericParameters);
		}

		public override void ConvertToLIR(LIRMethod pLIRMethod) { }

		protected override void DumpDetails(IndentableStreamWriter pWriter)
		{
			pWriter.WriteLine("Type {0}", Type.ToString());
		}

		public override string ToString()
		{
			return "SizeOf " + Type + " -> " + Destination;
		}
	}
}
