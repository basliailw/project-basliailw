# M5 Design

## 1. M4 fixes
The AIWS_FAULT_LOSE_FAILURE block in CorpusBuilderWorker::run() was missing a }, so the faulted build did not compile. That M4 fault is removed. The E3 test now checks the return values of ::write and ::read. M4's DESIGN.md did not declare the worker's result data or the IPC thread; both are declared below.

## 2. Threads and shared data
The main GUI thread runs MainWindow, state objects and widgets. The build thread is created once building has been initiated. It runs CorpusBuilderWorker::run(), and is joined when shutdown() is called. The IPC thread runs IpcServer::run() and is joined when IpcServer::stop() is called. The main GUI thread sets the request_, has_request_ and stop_ members. The build thread sets the result_, error_ and succeeded_ members. All of these 6 members are protected with mutex_. The build thread waits on wake_ until a request or stop is made. The cancel_requested_ member (set by main GUI thread, read by build thread) and IpcServer::stop_requested_ (set by main GUI thread, read by IPC thread) are std::atomics. busy_ and state_ are only accessed by the GUI thread. The build thread creates its own Workspace.

## 3. States
| State           | Meaning                 | Build requested | Workspace changed   | Build succeeded | Build failed |
|-----------------|-------------------------|-----------------|---------------------|-----------------|--------------|
| NotBuiltState   | no current corpus       | → Building      | stay                | —               | —            |
| BuildingState   | wanted build running    | —               | cancel → Cancelling | → Current       | → NotBuilt   |
| CancellingState | unwanted build stopping | —               | stay                | → NotBuilt      | → NotBuilt   |
| CurrentState    | corpus is current       | —               | → NotBuilt          | —               | —            |

Add, remote add and Reset all send "workspace changed". Each event returns the next state, which MainWindow::change_state() installs.

## 4. Event sequence: Reset during a long build
GUI calls build_corpus() which causes NotBuiltState::build_requested() to be called. This causes start_build() to be called. CorpusBuilderWorker::start() sets request_, and calls notify_one(). State changes to Building. Worker is notified, receives request, and waits in wait_until() until the first document is available. User presses Reset. reset_application() causes BuildingState::workspace_changed() to be called. This calls cancel_build(), causing cancel_requested_ to be set, and notifies. State changes to Cancelling. The worker is notified early, does not call rebuild(), but calls build_finished. In the GUI, build_finished() calls finish() and CancellingState::build_failed(). build_failed() returns NotBuilt, so the Build button is almost immediately re-enabled.

## 5. Decisions
Building during a build is not allowed: only NotBuiltState::can_build() returns true. A remote add mid-build goes through remote_add_requested() and add_document_file(), so BuildingState::workspace_changed() cancels the build; the user sees "Not built" and can rebuild right away. The smallest shared data is the request, the result, and the stop and cancel flags, because the GUI must hand over work, receive a result, and be able to stop the worker.

## 6. Fault switches
AIWS_FAULT_NO_CANCEL makes CorpusBuilderWorker::cancel() return without cancelling; "M5 R3: Reset mid-build stops the useless build quickly" catches it. AIWS_FAULT_SILENT_SHUTDOWN makes CorpusBuilderWorker::shutdown() skip notify_one(), so an idle worker never wakes; "M5 R4: exit with an idle worker is fast" hangs and the 30 s CTest timeout fails it.

## 7. AI-use disclosure
When searching Google for syntax questions (looking for forums like Stack Overflow), Google's automatic AI Overview provided certain syntax explanations and examples. I did not prompt a standalone AI chatbot.