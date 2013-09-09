import java.util.LinkedList;
import java.util.List;
import java.util.Random;


public class BRGenerator {

	private static int noatoms = 0;
	private static int noclausesa = 0;
	private static int noclausesb = 0;
	private static double factor = 0.3;
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		if (args.length < 3 || args.length > 4) {
			System.err.println(usage());
			System.exit(1);
		}
		
		try {
			noatoms = Integer.valueOf(args[0]);
			noclausesa = Integer.valueOf(args[1]);
			noclausesb = Integer.valueOf(args[2]);
			if (args.length == 4)
				factor = Double.valueOf(args[3]);
			
		} catch (Exception e) {
			System.err.println(e.getMessage());
			System.err.println(usage());
			System.exit(1);
		}
		
		StringBuilder stb = new StringBuilder();
		
		for (int i = 1; i <= noatoms; i++) {
			stb.append("atom(").append(i).append(").\n");
		}
		stb.append("\n");
		for (int i = 1; i <= noclausesa; i++) {
			stb.append("clausea(").append(i).append(").\n");
		}
		stb.append("\n");
		for (int i = noclausesa + 1; i <= noclausesa + noclausesb; i++) {
			stb.append("clauseb(").append(i).append(").\n");
		}
		stb.append("\n");
		
		// fill clauses
		Random random = new Random();
		for (int clause = 1; clause <= noclausesa + noclausesb; clause++) {
			int numberOfAtomsInClause = random.nextInt((int) (noatoms * factor)) + 1;
			if (numberOfAtomsInClause > noatoms) {
				numberOfAtomsInClause = noatoms;
			}
			List<Integer> selectedAtoms = new LinkedList<Integer>();
			while (selectedAtoms.size() < numberOfAtomsInClause) {
				int atom = random.nextInt(noatoms) +1;
				if (selectedAtoms.contains(atom))
					continue;
				selectedAtoms.add(atom);
			}
			
			for (Integer atom : selectedAtoms) {
				boolean positive = random.nextBoolean();
			
				if (positive) {
					stb.append("pos(");
				} else {
					stb.append("neg(");
				}
				stb.append(atom)
				.append(",")
				.append(clause)
				.append(").\n");
			}
			stb.append("\n");
		}
		stb.append("\n");
		
		System.out.println(stb.toString());
	}

	public static String usage() {
		StringBuilder stb = new StringBuilder();
		
		stb.append("Usage: java BRGenerator <noatoms> <noclausesa> <noclausesb> <factor?>\n");
		stb.append("\t<noatoms>   \t Overall number of atoms\n");
		stb.append("\t<noclausesa>\t Overall number of clauses in alpha\n");
		stb.append("\t<noclausesb>\t Overall number of clauses in beta\n");
		stb.append("\t<factor?>   \t <noatoms>*<factor> gives the estimated number of atoms in clauses. Default: ");
		stb.append(factor).append("\n");
		
		return stb.toString();
	}
}
