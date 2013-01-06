﻿using System;
using System.Collections.Generic;

namespace Proton.LIR.Instructions
{
	public sealed class Call : LIRInstruction
	{
		public LIRMethod TargetMethod { get; private set; }
		private List<ISource> mSources = new List<ISource>();
		public List<ISource> Sources { get { return mSources; } }
		public IDestination ReturnValueDestination { get; private set; }

		public Call(LIRMethod parent, LIRMethod targetMethod, IEnumerable<ISource> sources = null, IDestination returnValueDest = null) : base(parent)
		{
			this.TargetMethod = targetMethod;
			if (sources != null)
				Sources.AddRange(sources);
			this.ReturnValueDestination = returnValueDest;
		}

		internal override void Dump(IndentedStreamWriter wtr)
		{
			wtr.Write("Call {0}({1})", TargetMethod, String.Join(", ", Sources));
			if (ReturnValueDestination != null)
				wtr.Write(" -> {0}", ReturnValueDestination);
			wtr.WriteLine();
		}
	}
}
