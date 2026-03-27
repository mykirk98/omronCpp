# plan_guideline.md
 
This document defines the behavioral rules for Claude Code when reading and executing `plan.md`.
Anyone writing `plan.md` must follow the structure and rules below.
 
---
 
## 1. Structure of plan.md
 
```markdown
## Current Status
- Summary of what has been completed so far
- Current branch, failing tests, or other relevant context
 
## Task List
- [x] Completed task
- [ ] Next task to execute       ← Claude Code executes this one only
- [ ] Task after that
```
 
---
 
## 2. Execution Rules for Claude Code
 
### 2-1. One task at a time
- Execute only the **first unchecked** `- [ ]` item in the list.
- Never move on to the next task before the current one is complete.
 
### 2-2. All completion criteria must be met
- If a task includes `Acceptance Criteria:`, every criterion must be satisfied before the task is considered done.
- If no criteria are listed, use the task description itself as the standard.
 
### 2-3. Stop after completion
- When a task is done, update `- [ ]` to `- [x]`.
- Then **stop immediately** and wait for human review.
- Do not start the next task on your own.
 
### 2-4. Stay within scope
- Do not modify any file, module, or feature not mentioned in the task.
- Do not touch code that "seems like it could use improvement" if it is outside the task scope.
- If out-of-scope work appears necessary, report it to the human instead of acting on it.
 
### 2-5. Separate structural and behavioral changes
- Do not mix refactoring (structural change) with feature additions (behavioral change) in the same commit.
- If a task seems to require both, ask the human whether to split them before proceeding.

---

## 3. plan.md 작성 규칙 (사람을 위한 가이드)

### 3-1. 태스크 크기
- 태스크 하나는 **TDD 한 사이클(Red → Green → Refactor)** 안에 끝날 수 있는 크기로 작성한다.
- 30분~1시간 이내로 완료 가능한 크기가 적절하다.
- 크면 쪼갠다.

### 3-2. 완료 조건 명시
```markdown
- [ ] FrameBuffer 클래스 추가
  - 완료 조건: FrameBufferTests 전체 통과, IDisposable 구현 포함
```

### 3-3. 금지 사항 명시
```markdown
- [ ] MainViewModel에 프로퍼티 추가
  - ⚠️ 금지: UI 코드비하인드 수정
  - ⚠️ 금지: 외부 라이브러리 신규 추가
```

### 3-4. 의존성 순서
- 의존하는 태스크가 있으면 선행 태스크를 먼저 배치한다.
- 필요하면 주석으로 명시한다.
```markdown
- [ ] 2. ICameraService에 이벤트 추가  ← 1번 완료 후 실행
```

### 3-5. 현재 상태 섹션 유지
- 매 태스크 완료 후 `## 현재 상태` 섹션을 최신으로 유지한다.
- Claude Code는 새 세션마다 이 섹션을 컨텍스트로 활용한다.

---

## 4. What Claude Code Must Never Do
 
| Prohibited Action | Reason |
|---|---|
| Execute multiple tasks at once | Removes the human's opportunity to review |
| Mark a task complete before all criteria are met | Creates a false record of progress |
| Add tasks to plan.md without human instruction | Direction must remain with the human |
| Start the next task automatically | Expands scope without human confirmation |
| Reorder tasks arbitrarily | May break dependency order |