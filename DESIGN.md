# M0 Design and Understanding Note

Answer briefly in your own words. This is not intended to be a long report.

1. What responsibility belongs to `Workspace`, and what responsibilities belong to `Document`, `Prompt`, and `Message` instead?

Workspace - Holds the name of Documents, Prompts, and Messages. Coordinates ownership and access to the other 3 types.
Dcoument  - Owns everything about one text document (title, path, contents). Can load itself and report simple facts.
Prompt    - A plain value object for a reusable instruction (title and instruction text).
Message   - Plain value object for a reusable instruction. Stores and compares data.

2. Why are the collections inside `Workspace` private? Explain the purpose of the const and non-const `At` overloads.
    It forces all access through Workspace's own methods, so nothing outside the class can reorder or change the vectors directly and break invariants like insertion order. The purpose of the const and non-const 'At' overloads let the compiler pick automatically. A mutable Workspace gets a modifiable reference back whereas a const Workspace gets a read-only one, enforced at compile time.

3. Explain one meaningful test you added. What behavior does it check, and what implementation error could it catch?
    The Document::load() success test checks that title(), sourcePath(), and charactercount() all come out correct after loading a real file. It catches 2 bugs: forgetting to extract the filename for title_ and a getline-based read that silently dropped a trailing newline instead of reproducing the file exactly.

4. Describe one implementation decision that you verified, tested, or revised before submitting your work.
    I revised how Document::load() reads a file's contents after realizing a getline-based, line-by-line reconstruction doesn't preserve trailing newlines or /r/n endings exactly. I switched to reading the raw bytes directly and verified it by loading a file with a deliberate trailing newline and checking contents() matched exactly.

5. If generative AI was used, disclose it as required by course policy. If no generative AI was used, state that. The disclosure itself is not used as proof of authorship or understanding.
    Due to me misunderstanding the rubric, I used AI to summarize the rubric and make a list of deliverables that is expected of me for M0. AI was not used for coding.