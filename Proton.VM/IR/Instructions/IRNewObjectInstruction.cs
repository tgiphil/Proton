using System;
using System.Collections.Generic;

namespace Proton.VM.IR.Instructions
{
    public sealed class IRNewObjectInstruction : IRInstruction
    {
		private IRMethod mConstructor = null;
		public IRMethod Constructor { get { return mConstructor; } private set { mConstructor = value; } }

        public IRNewObjectInstruction(IRMethod pConstructor) : base(IROpcode.NewObject) { Constructor = pConstructor; }

        public override void Linearize(Stack<IRStackObject> pStack)
        {
            // Skip first parameter source, it is the 'this' reference that is put on the stack by newobj before constructor call
            for (int count = 1; count < Constructor.Parameters.Count; ++count) Sources.Add(new IRLinearizedLocation(pStack.Pop().LinearizedTarget));

            IRStackObject result = new IRStackObject();
            result.Type = Constructor.ParentType;
            result.LinearizedTarget = new IRLinearizedLocation(IRLinearizedLocationType.Local);
            result.LinearizedTarget.Local.LocalIndex = AddLinearizedLocal(pStack, Constructor.ParentType);
            Destination = new IRLinearizedLocation(result.LinearizedTarget);
            pStack.Push(result);
        }

        public override IRInstruction Clone(IRMethod pNewMethod) { return CopyTo(new IRNewObjectInstruction(Constructor), pNewMethod); }

		public override bool Resolved { get { return Constructor.Resolved; } }
		public override void Resolve()
		{
			base.Resolve();
			Constructor.Resolve(ref mConstructor, ParentMethod.ParentType.GenericParameters, ParentMethod.GenericParameters);
		}

		protected override void DumpDetails(IndentableStreamWriter pWriter)
		{
			pWriter.WriteLine("Constructor {0}", Constructor.ToString());
		}
	}
}