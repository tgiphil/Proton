﻿using System;
using System.Collections.Generic;
using Proton.VM.IR.Instructions;

namespace Proton.VM.IR.Optimizations
{
	public sealed class IRIndirectionRemovalOptimizationPass : IROptimizationPass
	{
		public override string Name { get { return "Indirection Removal"; } }
		public override string Description { get { return "Removes indirect accesses to values at a calculated address and uses the value instead."; } }
		public override IROptimizationPass.RunLocation Location { get { return RunLocation.DuringSSA; } }

		private static void ProcessIndirection(IRLinearizedLocation loc)
		{
			switch (loc.Type)
			{
				case IRLinearizedLocationType.Null: break;
				case IRLinearizedLocationType.Local: break;
				case IRLinearizedLocationType.LocalAddress: break;
				case IRLinearizedLocationType.Parameter: break;
				case IRLinearizedLocationType.ParameterAddress: break;
				case IRLinearizedLocationType.ConstantI4: break;
				case IRLinearizedLocationType.ConstantI8: break;
				case IRLinearizedLocationType.ConstantR4: break;
				case IRLinearizedLocationType.ConstantR8: break;
				case IRLinearizedLocationType.RuntimeHandle: break;
				case IRLinearizedLocationType.String: break;
				case IRLinearizedLocationType.StaticField: break;
				case IRLinearizedLocationType.StaticFieldAddress: break;
				case IRLinearizedLocationType.Indirect:
					var adrLoc = loc.Indirect.AddressLocation;
					switch (adrLoc.Type)
					{
						case IRLinearizedLocationType.ArrayElementAddress:
							loc.Type = IRLinearizedLocationType.ArrayElement;
							loc.ArrayElement.ArrayLocation = adrLoc.ArrayElementAddress.ArrayLocation;
							loc.ArrayElement.IndexLocation = adrLoc.ArrayElementAddress.IndexLocation;
							loc.ArrayElement.ElementType = adrLoc.ArrayElementAddress.ElementType;
							ProcessIndirection(loc.ArrayElement.ArrayLocation);
							ProcessIndirection(loc.ArrayElement.IndexLocation);
							break;
						case IRLinearizedLocationType.FieldAddress:
							loc.Type = IRLinearizedLocationType.Field;
							loc.Field.Field = adrLoc.FieldAddress.Field;
							loc.Field.FieldLocation = adrLoc.FieldAddress.FieldLocation;
							ProcessIndirection(loc.Field.FieldLocation);
							break;
						case IRLinearizedLocationType.LocalAddress:
							loc.Type = IRLinearizedLocationType.Local;
							loc.Local.LocalIndex = adrLoc.LocalAddress.LocalIndex;
							break;
						case IRLinearizedLocationType.ParameterAddress:
							loc.Type = IRLinearizedLocationType.Parameter;
							loc.Parameter.ParameterIndex = adrLoc.ParameterAddress.ParameterIndex;
							break;
						case IRLinearizedLocationType.StaticFieldAddress:
							loc.Type = IRLinearizedLocationType.StaticField;
							loc.StaticField.Field = adrLoc.StaticFieldAddress.Field;
							break;
						default:
							break;
					}
					break;
				case IRLinearizedLocationType.Field:
					ProcessIndirection(loc.Field.FieldLocation);
					break;
				case IRLinearizedLocationType.FieldAddress:
					ProcessIndirection(loc.FieldAddress.FieldLocation);
					break;
				case IRLinearizedLocationType.ArrayElement:
					ProcessIndirection(loc.ArrayElement.ArrayLocation);
					ProcessIndirection(loc.ArrayElement.IndexLocation);
					break;
				case IRLinearizedLocationType.ArrayElementAddress:
					ProcessIndirection(loc.ArrayElementAddress.ArrayLocation);
					ProcessIndirection(loc.ArrayElementAddress.IndexLocation);
					break;
				case IRLinearizedLocationType.ArrayLength:
					ProcessIndirection(loc.ArrayLength.ArrayLocation);
					break;
				case IRLinearizedLocationType.FunctionAddress:
					if (loc.FunctionAddress.InstanceLocation != null)
						ProcessIndirection(loc.FunctionAddress.InstanceLocation);
					break;
				case IRLinearizedLocationType.Phi:
					loc.Phi.SourceLocations.ForEach(s => ProcessIndirection(s));
					break;
				case IRLinearizedLocationType.SizeOf:
					throw new Exception("This shouldn't exist!");
				default:
					throw new Exception("Unknown IRLinearizedLocationType");
			}
		}

		public override void Run(IRMethod pMethod)
		{
			for (int i = 0; i < pMethod.Instructions.Count - 1; i++)
			{
				var curInstr = pMethod.Instructions[i];
				curInstr.Sources.ForEach(s => ProcessIndirection(s));
				if (curInstr.Destination != null)
					ProcessIndirection(curInstr.Destination);
			}
		}
	}
}
