using Proton.LIR;
using LIRInstructions = Proton.LIR.Instructions;
using System;
using System.Collections.Generic;

namespace Proton.VM.IR.Instructions
{
    public sealed class IRLoadParameterInstruction : IRInstruction
    {
        public uint ParameterIndex { get; private set; }

        public IRLoadParameterInstruction(uint pParameterIndex) : base(IROpcode.LoadParameter) { ParameterIndex = pParameterIndex; }

        public override void Linearize(Stack<IRStackObject> pStack)
        {
			IRLinearizedLocation source = new IRLinearizedLocation(this, IRLinearizedLocationType.Parameter);
            source.Parameter.ParameterIndex = ParameterIndex;
            Sources.Add(source);

            IRStackObject result = new IRStackObject();
            result.Type = ParentMethod.Parameters[(int)ParameterIndex].Type;
			result.LinearizedTarget = new IRLinearizedLocation(this, IRLinearizedLocationType.Local);
            result.LinearizedTarget.Local.LocalIndex = AddLinearizedLocal(pStack, ParentMethod.Parameters[(int)ParameterIndex].Type);
			Destination = new IRLinearizedLocation(this, result.LinearizedTarget);
            pStack.Push(result);
        }

        public override IRInstruction Clone(IRMethod pNewMethod) { return CopyTo(new IRLoadParameterInstruction(ParameterIndex), pNewMethod); }

        public override IRInstruction Transform() { return new IRMoveInstruction(this); }

		public override void ConvertToLIR(LIRMethod pLIRMethod) { }

		protected override void DumpDetails(IndentableStreamWriter pWriter)
		{
			pWriter.WriteLine("ParameterIndex {0}", ParameterIndex);
		}
	}
}
