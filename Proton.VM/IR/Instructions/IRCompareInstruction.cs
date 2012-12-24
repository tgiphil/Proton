using System;
using System.Collections.Generic;

namespace Proton.VM.IR.Instructions
{
    public sealed class IRCompareInstruction : IRInstruction
    {
        private IRCompareCondition mCompareCondition = IRCompareCondition.Equal;
        public IRCompareCondition CompareCondition
        {
            get { return mCompareCondition; }
            private set { mCompareCondition = value; }
        }

        public IRCompareInstruction(IRCompareCondition pCompareCondition) : base(IROpcode.Compare) { CompareCondition = pCompareCondition; }

        public override void Linearize(Stack<IRStackObject> pStack)
        {
            IRStackObject value2 = pStack.Pop();
            IRStackObject value1 = pStack.Pop();

			Sources.Add(new IRLinearizedLocation(this, value1.LinearizedTarget));
			Sources.Add(new IRLinearizedLocation(this, value2.LinearizedTarget));

            IRStackObject result = new IRStackObject();
            result.Type = ParentMethod.Assembly.AppDomain.System_Int32;
			result.LinearizedTarget = new IRLinearizedLocation(this, IRLinearizedLocationType.Local);
            result.LinearizedTarget.Local.LocalIndex = AddLinearizedLocal(pStack, ParentMethod.Assembly.AppDomain.System_Int32);
			Destination = new IRLinearizedLocation(this, result.LinearizedTarget);
            pStack.Push(result);
        }

        public override IRInstruction Clone(IRMethod pNewMethod) { return CopyTo(new IRCompareInstruction(CompareCondition), pNewMethod); }

		protected override void DumpDetails(IndentableStreamWriter pWriter)
		{
			pWriter.WriteLine("CompareCondition {0}", CompareCondition.ToString());
		}
	}
}
